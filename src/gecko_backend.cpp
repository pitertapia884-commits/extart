#include "gecko_backend.hpp"

#include <system_error>

bool GeckoBackend::configure(const std::filesystem::path& runtime_root) {
    runtime_root_.clear();

    if (runtime_root.empty()) return false;

    std::error_code error;
    const auto absolute_root = std::filesystem::absolute(runtime_root, error);
    if (error || !std::filesystem::is_directory(absolute_root, error) || error) {
        return false;
    }

    runtime_root_ = absolute_root;
    return true;
}
