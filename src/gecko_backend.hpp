#pragma once

#include <filesystem>
#include <string>

// Native Gecko integration boundary for EXTART.
//
// This class describes the Gecko runtime that EXTART will embed. It does not
// launch Firefox and it does not treat an external Firefox process as an
// embedded engine.
class GeckoBackend {
public:
    GeckoBackend() = default;
    ~GeckoBackend() = default;

    GeckoBackend(const GeckoBackend&) = delete;
    GeckoBackend& operator=(const GeckoBackend&) = delete;

    bool configure(const std::filesystem::path& runtime_root);
    bool configured() const { return !runtime_root_.empty(); }

    const std::filesystem::path& runtime_root() const { return runtime_root_; }

private:
    std::filesystem::path runtime_root_;
};
