#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>

namespace
{
    constexpr auto loopParamId = "loopLevel";

    /** Safety cap so a full movie does not load entirely into RAM (~15 min of audio). */
    constexpr double kMaxLoopDurationSec = 900.0;

    juce::Array<juce::File> getLoopFileCandidates()
    {
        juce::Array<juce::File> candidates;

        const auto env = juce::SystemStats::getEnvironmentVariable ("TRUCK_PACKER_LOOP", {}).trim();
        if (env.isNotEmpty())
            candidates.add (juce::File (env));

        const auto home = juce::File::getSpecialLocation (juce::File::userHomeDirectory);
        const juce::File dirs[] = {
            home.getChildFile ("Downloads"),
            juce::File::getSpecialLocation (juce::File::userDesktopDirectory),
            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
            juce::File::getSpecialLocation (juce::File::userMusicDirectory),
            juce::File::getSpecialLocation (juce::File::userMoviesDirectory),
        };

        for (auto dir : dirs)
        {
            candidates.add (dir.getChildFile ("videoplayback.mp4"));
            candidates.add (dir.getChildFile ("videoplayback.wav"));
            candidates.add (dir.getChildFile ("videoplayback.mp3"));
        }

        return candidates;
    }
}

TruckPackerWrapperAudioProcessor::TruckPackerWrapperAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    , apvts (*this,
             nullptr,
             "PARAMS",
             { std::make_unique<juce::AudioParameterFloat> (
                   loopParamId,
                   "Inspirational Music",
                   juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f, 0.5f),
                   0.0f,
                   juce::AudioParameterFloatAttributes()
                       .withLabel ("%")
                       .withStringFromValueFunction ([] (float v, int) {
                           return juce::String (juce::roundToInt (v * 100.0f)) + " %";
                       })
                       .withValueFromStringFunction ([] (const juce::String& text) {
                           return juce::jlimit (0.0f, 1.0f, text.getFloatValue() / 100.0f);
                       })) })
{
    loopGainSmoothed.reset (44100.0, 0.04);
    loopGainSmoothed.setCurrentAndTargetValue (0.0f);
    tryLoadLoopFile();
}

TruckPackerWrapperAudioProcessor::~TruckPackerWrapperAudioProcessor() = default;

void TruckPackerWrapperAudioProcessor::tryLoadLoopFile()
{
    loopBuffer.setSize (0, 0);
    loopLengthSamples = 0;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    for (const auto& file : getLoopFileCandidates())
    {
        if (! file.existsAsFile())
            continue;

        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
        if (reader == nullptr)
        {
            if (auto in = file.createInputStream())
                reader.reset (formatManager.createReaderFor (std::move (in)));
        }

        if (reader == nullptr)
            continue;

        const auto maxSamples = (juce::int64) (reader->sampleRate * kMaxLoopDurationSec);
        const auto numSamples = juce::jmin (reader->lengthInSamples, maxSamples);
        if (numSamples <= 0)
            continue;

        const int ch = juce::jlimit (1, 2, (int) reader->numChannels);
        loopBuffer.setSize (ch, (int) numSamples, false, true, true);

        if (! reader->read (&loopBuffer, 0, (int) numSamples, 0, true, true))
        {
            loopBuffer.setSize (0, 0);
            continue;
        }

        loopFileSampleRate = reader->sampleRate;
        loopLengthSamples = (int) numSamples;
        readPosition = 0.0;
        return;
    }
}

float TruckPackerWrapperAudioProcessor::readLoopInterpolated (int channel, double position) const noexcept
{
    const int n = loopLengthSamples;
    const int nc = loopBuffer.getNumChannels();
    if (n <= 1 || nc < 1)
        return 0.0f;

    const int ch = juce::jmin (channel, nc - 1);

    while (position < 0.0)
        position += (double) n;

    while (position >= (double) n)
        position -= (double) n;

    const int i0 = (int) position;
    const float frac = (float) (position - (double) i0);
    const int i1 = (i0 + 1 >= n) ? 0 : i0 + 1;

    const float s0 = loopBuffer.getSample (ch, i0);
    const float s1 = loopBuffer.getSample (ch, i1);
    return s0 + frac * (s1 - s0);
}

void TruckPackerWrapperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    hostSampleRate = juce::jmax (8000.0, sampleRate);
    loopGainSmoothed.reset (hostSampleRate, 0.04);

    if (auto* raw = apvts.getRawParameterValue (loopParamId))
        loopGainSmoothed.setCurrentAndTargetValue (raw->load());

    readPosition = 0.0;

    if (loopLengthSamples <= 0)
        tryLoadLoopFile();
}

void TruckPackerWrapperAudioProcessor::releaseResources() {}

bool TruckPackerWrapperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    return mainOutput == juce::AudioChannelSet::mono()
           || mainOutput == juce::AudioChannelSet::stereo();
}

void TruckPackerWrapperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                     juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    midi.clear();

    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0)
        return;

    const int totalOut = getTotalNumOutputChannels();

    for (int ch = 0; ch < totalOut; ++ch)
        buffer.clear (ch, 0, numSamples);

    if (auto* raw = apvts.getRawParameterValue (loopParamId))
        loopGainSmoothed.setTargetValue (raw->load());

    auto* l = totalOut > 0 ? buffer.getWritePointer (0) : nullptr;
    auto* r = totalOut > 1 ? buffer.getWritePointer (1) : nullptr;

    if (l == nullptr)
        return;

    const bool haveLoop = loopLengthSamples > 0;
    const double step = haveLoop ? (loopFileSampleRate / hostSampleRate) : 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        const float g = loopGainSmoothed.getNextValue();

        if (haveLoop)
        {
            const float sl = readLoopInterpolated (0, readPosition);
            const float sr = loopBuffer.getNumChannels() > 1 ? readLoopInterpolated (1, readPosition) : sl;

            if (r != nullptr && r != l)
            {
                l[i] += g * sl;
                r[i] += g * sr;
            }
            else
            {
                const float mono = loopBuffer.getNumChannels() > 1 ? 0.5f * (sl + sr) : sl;
                l[i] += g * mono;
            }

            readPosition += step;
            while (readPosition >= (double) loopLengthSamples)
                readPosition -= (double) loopLengthSamples;
        }
    }
}

juce::AudioProcessorEditor* TruckPackerWrapperAudioProcessor::createEditor()
{
    return new TruckPackerWrapperAudioProcessorEditor (*this);
}

void TruckPackerWrapperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        juce::AudioProcessor::copyXmlToBinary (*xml, destData);
}

void TruckPackerWrapperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = juce::AudioProcessor::getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TruckPackerWrapperAudioProcessor();
}
