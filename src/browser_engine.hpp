#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <memory>
#include <string>

class Config;
class DownloadManager;
class Profile;

class BrowserEngine {
public:
    struct Callbacks {
        std::function<void(const char*)> uri_changed;
        std::function<void(const char*)> title_changed;
        std::function<void()> load_finished;
    };

    virtual ~BrowserEngine() = default;

    BrowserEngine(const BrowserEngine&) = delete;
    BrowserEngine& operator=(const BrowserEngine&) = delete;

    virtual GtkWidget* widget() const = 0;
    virtual gpointer native_handle() const = 0;
    virtual void set_callbacks(Callbacks callbacks) = 0;

    virtual std::string current_uri() const = 0;
    virtual std::string current_title() const = 0;
    virtual bool can_go_back() const = 0;
    virtual bool can_go_forward() const = 0;
    virtual void go_back() = 0;
    virtual void go_forward() = 0;
    virtual void reload() = 0;

    virtual void load_html(const std::string& html, const char* base_uri) = 0;
    virtual void load_uri(const char* uri) = 0;
    virtual void stop_loading() = 0;

    virtual void find_text(const std::string& text) = 0;
    virtual void find_next() = 0;
    virtual void find_previous() = 0;
    virtual void clear_find() = 0;

    virtual void apply_config(const Config& config) = 0;
    virtual void setup_downloads(DownloadManager& manager) = 0;
    virtual void clear_site_data() = 0;

protected:
    BrowserEngine() = default;
};

std::unique_ptr<BrowserEngine> make_browser_engine(Profile& profile, const Config& config);

