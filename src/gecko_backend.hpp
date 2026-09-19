#pragma once

#include <filesystem>
#include <string>

class GeckoBackend {
public:
    GeckoBackend() = default;
    ~GeckoBackend() = default;

    GeckoBackend(const GeckoBackend&) = delete;
    GeckoBackend& operator=(const GeckoBackend&) = delete;

    // Configure the directory containing the Gecko runtime resources.
    // This validates the runtime boundary; it never launches Firefox.
    bool configure(const std::filesystem::path& runtime_root);

    // Resolve the runtime from EXTART_GECKO_ROOT when explicitly provided.
    bool configure_from_environment();

    bool configured() const { return !runtime_root_.empty(); }

    const std::filesystem::path& runtime_root() const { return runtime_root_; }

    // The Gecko build layout used by EXTART places runtime artifacts below
    // obj-extart/dist. These helpers keep that layout out of GeckoEngine.
    std::filesystem::path runtime_binary_dir() const;
    std::filesystem::path runtime_resource_dir() const;

    const std::string& last_error() const { return last_error_; }

private:
    std::filesystem::path runtime_root_;
    std::string last_error_;
};
