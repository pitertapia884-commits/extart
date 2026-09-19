#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <string>

class Config;
class DownloadManager;
class Profile;

// Engine-neutral browser surface used by EXTART's GTK UI.
//
// The UI must not know whether the page renderer is WebKitGTK, Gecko, or
// another implementation. Native Gecko embedding will implement this same
// boundary later.
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

    virtual void load_html(const std::string& html, const char* base_uri) = 0;
    virtual void load_uri(const char* uri) = 0;
    virtual void stop_loading() = 0;

    virtual void find_text(const std::string& text) = 0;
    virtual void find_next() = 0;
    virtual void find_previous() = 0;
    virtual void clear_find() = 0;

    virtual void apply_config(const Config& config) = 0;
    virtual void setup_downloads(DownloadManager& manager) = 0;

protected:
    BrowserEngine() = default;
};

// Temporary implementation used while EXTART is being migrated away from
// WebKitGTK. It keeps all WebKit-specific code outside Tab.
class WebKitEngine final : public BrowserEngine {
public:
    WebKitEngine(Profile& profile, const Config& config);
    ~WebKitEngine() override;

    GtkWidget* widget() const override;
    gpointer native_handle() const override;
    void set_callbacks(Callbacks callbacks) override;

    void load_html(const std::string& html, const char* base_uri) override;
    void load_uri(const char* uri) override;
    void stop_loading() override;

    void find_text(const std::string& text) override;
    void find_next() override;
    void find_previous() override;
    void clear_find() override;

    void apply_config(const Config& config) override;
    void setup_downloads(DownloadManager& manager) override;

private:
    static void on_load_changed(struct _WebKitWebView* view,
                                 int event,
                                 gpointer user_data);

    GtkWidget* web_view_ = nullptr;
    struct _WebKitFindController* find_controller_ = nullptr;
    Callbacks callbacks_;
    std::string last_search_;
};
