#include "gecko_runtime.hpp"

#include <nsIAppShellService.h>
#include <nsIFile.h>
#include <nsIServiceManager.h>
#include <nsServiceManagerUtils.h>
#include <nsXPCOM.h>
#include <nsString.h>

GeckoRuntime::~GeckoRuntime() {
    // The runtime intentionally remains initialized for the process lifetime.
    // Individual windowless browsers are closed by GeckoEngine.
}

bool GeckoRuntime::initialize(const GeckoBackend& backend) {
    if (initialized()) {
        return true;
    }

    last_error_.clear();

    if (!backend.runtime_ready()) {
        last_error_ = "Gecko runtime artifacts are not ready";
        return false;
    }

    const auto bin_directory = backend.runtime_binary_dir();

    nsCOMPtr<nsIFile> bin_dir;
    nsresult rv = NS_NewNativeLocalFile(
        nsCString(bin_directory.c_str()), getter_AddRefs(bin_dir));
    if (NS_FAILED(rv) || !bin_dir) {
        last_error_ = "Unable to create Gecko binary directory object";
        return false;
    }

    rv = NS_InitXPCOM(&service_manager_, bin_dir, nullptr, true);
    if (NS_FAILED(rv) || !service_manager_) {
        service_manager_ = nullptr;
        last_error_ = "NS_InitXPCOM failed";
        return false;
    }

    nsCOMPtr<nsIAppShellService> app_shell =
        do_GetService("@mozilla.org/appshell/appShellService;1", &rv);
    if (NS_FAILED(rv) || !app_shell) {
        last_error_ = "Gecko app shell service is unavailable";
        return false;
    }

    app_shell_ = app_shell.get();
    app_shell_->AddRef();
    return true;
}
