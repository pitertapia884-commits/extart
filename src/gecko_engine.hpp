#pragma once

#include "browser_engine.hpp"
#include "gecko_backend.hpp"

#include <string>

class GeckoRuntime;
class nsIWindowlessBrowser;
class nsIWebNavigation;
class nsIWidget;

class GeckoEngine final : public BrowserEngine {
public:
    GeckoEngine(GeckoBackend& backend, Profile& profile, const Config& config);
    ~GeckoEngine() override;

    GtkWidget* widget() const override;
    gpointer native_handle() const override;
    void set_callbacks(Callbacks callbacks) override;

    std::string current_uri() const override;
    std::string current_title() const override;
    bool can_go_back() const override;
    bool can_go_forward() const override;
    void go_back() override;
    void go_forward() override;
    void reload() override;

    void load_html(const std::string& html, const char* base_uri) override;
    void load_uri(const char* uri) override;
    void stop_loading() override;

    void find_text(const std::string& text) override;
    void find_next() override;
    void find_previous() override;
    void clear_find() override;

    void apply_config(const Config& config) override;
    void setup_downloads(DownloadManager& manager) override;
    void clear_site_data() override;

private:
    GeckoBackend& backend_;
    GeckoRuntime* runtime_ = nullptr;
    nsIWindowlessBrowser* windowless_browser_ = nullptr;
    nsIWebNavigation* navigation_ = nullptr;
    nsIWidget* gecko_widget_ = nullptr;
    Profile& profile_;
    Callbacks callbacks_;
    std::string current_uri_;
    std::string current_title_;
    std::string last_search_;
};
