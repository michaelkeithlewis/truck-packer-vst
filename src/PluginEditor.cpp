#include "PluginEditor.h"
#include "TruckPackerWebView.h"

namespace
{
    constexpr auto kTruckPackerAppUrl = "https://app.truckpacker.com";
    constexpr auto loopParamId = "loopLevel";

    constexpr int kWebViewCreateDelayMs = 350;

    const juce::Colour kRailTop { 0xff2a3444 };
    const juce::Colour kRailBottom { 0xff4a2d6e };
    const juce::Colour kThumbFill { 0xff5ee1ff };
    const juce::Colour kThumbGlow { 0xff5ee1ff };
}

int LoopFaderLookAndFeel::getSliderThumbRadius (juce::Slider&)
{
    return 11;
}

void LoopFaderLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float minSliderPos, float maxSliderPos,
                                             juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearVertical)
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    juce::ignoreUnused (minSliderPos, maxSliderPos);

    const auto area = juce::Rectangle<int> (x, y, width, height).toFloat();
    const float cx = area.getCentreX();
    const float railW = juce::jmin (14.0f, area.getWidth() * 0.45f);
    const float pad = 10.0f;
    auto rail = juce::Rectangle<float> (cx - railW * 0.5f, area.getY() + pad, railW, area.getBottom() - area.getY() - 2.0f * pad);

    juce::ColourGradient grad (kRailTop, cx, rail.getY(), kRailBottom, cx, rail.getBottom(), false);
    grad.addColour (0.55, juce::Colour (0xff3a3f58));
    g.setGradientFill (grad);
    g.fillRoundedRectangle (rail, railW * 0.5f);

    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawRoundedRectangle (rail, railW * 0.5f, 1.2f);

    const float ty = juce::jlimit (rail.getY() + 2.0f, rail.getBottom() - 2.0f, sliderPos);
    const float thumbR = 13.0f;
    const auto thumb = juce::Rectangle<float> (cx - thumbR, ty - thumbR, thumbR * 2.0f, thumbR * 2.0f);

    g.setColour (kThumbGlow.withAlpha (0.28f));
    g.fillEllipse (thumb.expanded (6.0f));

    g.setColour (kThumbFill.withAlpha (0.95f));
    g.fillEllipse (thumb);

    g.setColour (juce::Colours::white.withAlpha (0.55f));
    g.fillEllipse (thumb.reduced (4.5f));

    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.drawEllipse (thumb, 1.2f);

    if (slider.isEnabled())
    {
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        const float notchY = rail.getCentreY();
        g.drawLine (rail.getX() + 2.0f, notchY, rail.getRight() - 2.0f, notchY, 1.0f);
    }
}

juce::Label* LoopFaderLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* l = juce::LookAndFeel_V4::createSliderTextBox (slider);
    l->setJustificationType (juce::Justification::centred);
    l->setFont (juce::FontOptions (13.0f).withKerningFactor (0.04f));
    l->setColour (juce::Label::textColourId, juce::Colours::whitesmoke);
    l->setColour (juce::Label::backgroundColourId, juce::Colour (0xcc1a2230));
    l->setColour (juce::Label::outlineColourId, juce::Colour (0xff5ee1ff).withAlpha (0.35f));
    l->setBorderSize ({ 4, 2, 4, 2 });
    return l;
}

TruckPackerWrapperAudioProcessorEditor::TruckPackerWrapperAudioProcessorEditor (
    TruckPackerWrapperAudioProcessor& p)
    : AudioProcessorEditor (&p)
{
    setOpaque (true);

    loopFader.setLookAndFeel (&loopFaderLnf);
    loopFader.setSliderStyle (juce::Slider::LinearVertical);
    loopFader.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 22);
    loopFader.setTooltip ("Loop: Downloads or Music folder — videoplayback.wav/.mp3/.mp4 (or set TRUCK_PACKER_LOOP). On Windows, WAV is most reliable. Instrument mode: audible when the fader is up without starting the timeline.");
    loopFader.setVelocityBasedMode (true);
    loopFader.setVelocityModeParameters (1.0, 1, 0.09, false);
    addAndMakeVisible (loopFader);

    loopAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        p.getValueTreeState(), loopParamId, loopFader);

    setSize (960, 640);
    setResizable (true, true);
    setResizeLimits (480, 360, 2000, 1400);
    resized();

    queueWebViewCreation();
}

TruckPackerWrapperAudioProcessorEditor::~TruckPackerWrapperAudioProcessorEditor()
{
    loopFader.setLookAndFeel (nullptr);
}

juce::Rectangle<int> TruckPackerWrapperAudioProcessorEditor::webViewBounds() const
{
    auto b = getLocalBounds();
    b.removeFromRight (faderStripWidth);
    return b;
}

void TruckPackerWrapperAudioProcessorEditor::queueWebViewCreation()
{
    if (webView != nullptr || webViewCreationQueued)
        return;

    webViewCreationQueued = true;

    juce::Component::SafePointer<TruckPackerWrapperAudioProcessorEditor> self (this);

    juce::MessageManager::callAsync ([self] {
        if (self == nullptr)
            return;

        juce::MessageManager::callAsync ([self] {
            if (self == nullptr)
                return;

            juce::Timer::callAfterDelay (kWebViewCreateDelayMs, [self] {
                if (self == nullptr)
                    return;

                self->webViewCreationQueued = false;
                self->createWebViewIfNeeded();
            });
        });
    });
}

void TruckPackerWrapperAudioProcessorEditor::createWebViewIfNeeded()
{
    if (webView != nullptr)
        return;

    webView = std::make_unique<TruckPackerWebView>();
    addAndMakeVisible (*webView);
    webView->toBack();
    resized();
    tryNavigateToApp();
}

void TruckPackerWrapperAudioProcessorEditor::tryNavigateToApp()
{
    if (appNavigationStarted || webView == nullptr)
        return;

    const auto wb = webViewBounds();
    if (wb.getWidth() < 32 || wb.getHeight() < 32)
        return;

    appNavigationStarted = true;

    const juce::String url (kTruckPackerAppUrl);
    juce::Component::SafePointer<TruckPackerWrapperAudioProcessorEditor> self (this);

    juce::MessageManager::callAsync ([self, url] {
        if (self == nullptr || self->webView == nullptr)
            return;

        self->webView->goToURL (url);
    });
}

void TruckPackerWrapperAudioProcessorEditor::visibilityChanged()
{
    juce::AudioProcessorEditor::visibilityChanged();
    queueWebViewCreation();
}

void TruckPackerWrapperAudioProcessorEditor::parentSizeChanged()
{
    juce::AudioProcessorEditor::parentSizeChanged();
    resized();
}

void TruckPackerWrapperAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0c10));

    auto faderArea = getLocalBounds().removeFromRight (faderStripWidth);
    g.setColour (juce::Colours::white.withAlpha (0.28f));
    g.setFont (juce::FontOptions (10.5f).withKerningFactor (0.12f));
    g.drawFittedText ("LOOP", faderArea.removeFromTop (16), juce::Justification::centred, 1);

    if (webView == nullptr)
    {
        auto mainArea = webViewBounds();
        g.setColour (juce::Colours::white.withAlpha (0.4f));
        g.setFont (juce::FontOptions (14.0f));
        g.drawFittedText ("Loading browser...", mainArea, juce::Justification::centred, 2);
    }
}

void TruckPackerWrapperAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto faderStrip = bounds.removeFromRight (faderStripWidth);
    loopFader.setBounds (faderStrip.reduced (8, 20).withTrimmedBottom (2));

    if (webView != nullptr)
    {
        const auto wb = webViewBounds();
        webView->setBounds (wb);

        if (wb.getWidth() > 0 && wb.getHeight() > 0)
            webView->repaint();
    }

    loopFader.toFront (false);

    queueWebViewCreation();
    tryNavigateToApp();
}
