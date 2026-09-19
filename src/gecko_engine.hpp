#pragma once

#include "browser_engine.hpp"
#include "gecko_backend.hpp"

#include <string>
#include <memory>
#include <cstddef>
#include <memory>
#include <cstddef>

class GeckoRuntime;
class nsIWindowlessBrowser;
class nsIWebNavigation;
class nsIWidget;
class GtkWidget;
namespace mozilla { namespace gfx { class DrawTarget; } }

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

    GtkWidget* render_widget_ = nullptr;
    std::unique_ptr<unsigned char[]> render_pixels_;
    std::size_t render_stride_ = 0;
    int render_width_ = 0;
    int render_height_ = 0;
    mozilla::gfx::DrawTarget* render_target_ = nullptr;
    unsigned int render_source_id_ = 0;

    void ensure_render_target(int width, int height);
    void render_frame();
    static void draw_render_surface(GtkDrawingArea* area, cairo_t* cr,
                                    int width, int height, gpointer user_data);
    static gboolean render_tick(gpointer user_data);
};
