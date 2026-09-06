#include "memory_manager.hpp"

#include <algorithm>
#include <vector>

namespace {
std::vector<WebKitWebView*>& views() {
    static std::vector<WebKitWebView*> list;
    return list;
}
}

MemoryManager& MemoryManager::instance() {
    static MemoryManager manager;
    return manager;
}

void MemoryManager::register_view(WebKitWebView* view) {
    if (!view) return;
    views().push_back(view);
}

void MemoryManager::unregister_view(WebKitWebView* view) {
    auto& list = views();
    list.erase(std::remove(list.begin(), list.end(), view), list.end());
}

void MemoryManager::cleanup_idle_resources() {
    // Reserved for the next step: tab sleeping.
    // We keep this empty for now to avoid breaking active sessions.
}
