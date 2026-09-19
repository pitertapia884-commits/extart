#include "gecko_backend.hpp"

#include <cstdlib>
#include <filesystem>

namespace {

bool is_executable_file(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path, error) &&
           !error &&
           ::access(path.c_str(), X_OK) == 0;
}

} // namespace

bool GeckoBackend::available() const {
    return !executable_.empty();
}

bool GeckoBackend::locate() {
    executable_.clear();

    const char* configured = std::getenv("EXTART_GECKO_RUNTIME");
    if (configured != nullptr && *configured != '\0') {
        if (is_executable_file(configured)) {
            executable_ = configured;
            return true;
        }
    }

    const char* path = std::getenv("PATH");
    if (path == nullptr) return false;

    std::string paths(path);
    std::size_t start = 0;
    while (start <= paths.size()) {
        const std::size_t end = paths.find(':', start);
        const std::string directory = paths.substr(
            start,
            end == std::string::npos ? std::string::npos : end - start
        );

        for (const char* candidate : {"firefox", "firefox-nightly"}) {
            std::filesystem::path executable = directory.empty()
                ? std::filesystem::path(candidate)
                : std::filesystem::path(directory) / candidate;
            if (is_executable_file(executable)) {
                executable_ = executable.string();
                return true;
            }
        }

        if (end == std::string::npos) break;
        start = end + 1;
    }

    return false;
}
