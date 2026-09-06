#pragma once

#include <webkit/webkit.h>

// Lightweight memory coordinator for future tab sleeping and cleanup.
// This intentionally avoids destroying active WebViews.
class MemoryManager {
public:
    static MemoryManager& instance();

    void register_view(WebKitWebView* view);
    void unregister_view(WebKitWebView* view);
    void cleanup_idle_resources();

private:
    MemoryManager() = default;
};
