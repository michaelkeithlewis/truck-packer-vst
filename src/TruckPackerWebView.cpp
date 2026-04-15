#include "TruckPackerWebView.h"

namespace
{
    juce::WebBrowserComponent::Options makeWebViewOptions()
    {
       #if JUCE_WINDOWS && JUCE_USE_WIN_WEBVIEW2
        return juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
            .withUserAgent (
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
                "Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0");
       #elif JUCE_MAC || JUCE_IOS
        return juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            // Many sites treat embedded default UA as non-browser; blank or challenge pages can result.
            .withUserAgent (
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) "
                "Version/17.2 Safari/605.1.15");
       #else
        return juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withUserAgent (
                "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
                "Chrome/120.0.0.0 Safari/537.36");
       #endif
    }
}

TruckPackerWebView::TruckPackerWebView()
    : juce::WebBrowserComponent (makeWebViewOptions())
{
}

void TruckPackerWebView::newWindowAttemptingToLoad (const juce::String& newURL)
{
    if (newURL.isNotEmpty())
        goToURL (newURL);
}
