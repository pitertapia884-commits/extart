#include "gecko_backend.hpp"

#include <system_error>

bool GeckoBackend::configure(const std::filesystem::path& runtime_root) {
    runtime_root_.clear();

    if (runtime_root.empty()) return false;

    std::error_code error;
    if (!std::filesystem::exists(runtime_root, error) || error) return false;
    if (!std::filesystem::is_directory(runtime_root, error) || error) return false;

    runtime_root_ = std::filesystem::absolute(runtime_root, error);
    if (error) {
        runtime_root_.clear();
        return false;
    }

    return true;
}
