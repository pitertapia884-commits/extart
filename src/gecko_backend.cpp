#include "gecko_backend.hpp"

#include <cstdlib>
#include <system_error>

bool GeckoBackend::configure(const std::filesystem::path& runtime_root) {
    runtime_root_.clear();
    last_error_.clear();

    if (runtime_root.empty()) {
        last_error_ = "Gecko runtime path is empty";
        return false;
    }

    std::error_code error;
    const auto absolute_root = std::filesystem::absolute(runtime_root, error);
    if (error) {
        last_error_ = "Unable to resolve Gecko runtime path";
        return false;
    }

    if (!std::filesystem::is_directory(absolute_root, error) || error) {
        last_error_ = "Gecko runtime path is not a directory";
        return false;
    }

    runtime_root_ = absolute_root;
    return true;
}

bool GeckoBackend::configure_from_environment() {
    const char* value = std::getenv("EXTART_GECKO_ROOT");
    if (value == nullptr || *value == '\0') {
        last_error_ = "EXTART_GECKO_ROOT is not set";
        return false;
    }

    return configure(std::filesystem::path(value));
}

std::filesystem::path GeckoBackend::runtime_binary_dir() const {
    if (runtime_root_.empty()) return {};
    return runtime_root_ / "bin";
}

std::filesystem::path GeckoBackend::runtime_resource_dir() const {
    if (runtime_root_.empty()) return {};
    return runtime_root_ / "resources";
}
