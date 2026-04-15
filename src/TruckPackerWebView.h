#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

/** WKWebView returns nil for window.open targets; load those URLs here so OAuth / 2FA flows work. */
class TruckPackerWebView : public juce::WebBrowserComponent
{
public:
    TruckPackerWebView();

    void newWindowAttemptingToLoad (const juce::String& newURL) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TruckPackerWebView)
};
