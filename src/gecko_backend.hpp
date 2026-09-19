#pragma once

#include <string>

// Gecko integration boundary for EXTART.
// Gecko is kept behind this small interface so the GTK4 UI does not depend on
// Firefox's desktop UI. The concrete runtime is configured separately.
class GeckoBackend {
public:
    GeckoBackend() = default;
    ~GeckoBackend() = default;

    GeckoBackend(const GeckoBackend&) = delete;
    GeckoBackend& operator=(const GeckoBackend&) = delete;

    bool available() const;
    bool locate();
    const std::string& executable() const { return executable_; }

private:
    std::string executable_;
};
