#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class TruckPackerWebView;

/** Custom vertical fader: rail + glow thumb (drawn in .cpp). */
class LoopFaderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    int getSliderThumbRadius (juce::Slider&) override;
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;
    juce::Label* createSliderTextBox (juce::Slider& slider) override;
};

class TruckPackerWrapperAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    static constexpr int faderStripWidth = 72;

    explicit TruckPackerWrapperAudioProcessorEditor (TruckPackerWrapperAudioProcessor&);
    ~TruckPackerWrapperAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void visibilityChanged() override;
    void parentSizeChanged() override;

private:
    void queueWebViewCreation();
    void createWebViewIfNeeded();
    void tryNavigateToApp();
    juce::Rectangle<int> webViewBounds() const;

    std::unique_ptr<TruckPackerWebView> webView;
    juce::Slider loopFader;
    LoopFaderLookAndFeel loopFaderLnf;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loopAttachment;

    bool webViewCreationQueued = false;
    bool appNavigationStarted = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TruckPackerWrapperAudioProcessorEditor)
};
