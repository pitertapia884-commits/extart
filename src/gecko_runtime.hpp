#pragma once

#include "gecko_backend.hpp"

#include <string>

struct nsIServiceManager;
class nsIAppShellService;

class GeckoRuntime {
public:
    GeckoRuntime() = default;
    ~GeckoRuntime();

    GeckoRuntime(const GeckoRuntime&) = delete;
    GeckoRuntime& operator=(const GeckoRuntime&) = delete;

    bool initialize(const GeckoBackend& backend);
    bool initialized() const { return service_manager_ != nullptr; }

    nsIAppShellService* app_shell() const { return app_shell_; }
    const std::string& last_error() const { return last_error_; }

private:
    nsIServiceManager* service_manager_ = nullptr;
    nsIAppShellService* app_shell_ = nullptr;
    std::string last_error_;
};
