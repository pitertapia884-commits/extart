#include "gecko_engine.hpp"

#include "config.hpp"
#include "download_manager.hpp"

#include <utility>

GeckoEngine::GeckoEngine(GeckoBackend& backend, Profile& profile, const Config& config)
    : backend_(backend), profile_(profile) {
    // Native Gecko initialization belongs here once EXTART is linked against
    // a pinned Mozilla source revision and its embedding libraries.
    (void)config;
}

GeckoEngine::~GeckoEngine() = default;

GtkWidget* GeckoEngine::widget() const { return nullptr; }
gpointer GeckoEngine::native_handle() const { return nullptr; }

void GeckoEngine::set_callbacks(Callbacks callbacks) {
    callbacks_ = std::move(callbacks);
}

std::string GeckoEngine::current_uri() const { return current_uri_; }
std::string GeckoEngine::current_title() const { return current_title_; }
bool GeckoEngine::can_go_back() const { return false; }
bool GeckoEngine::can_go_forward() const { return false; }
void GeckoEngine::go_back() {}
void GeckoEngine::go_forward() {}
void GeckoEngine::reload() {}
void GeckoEngine::load_html(const std::string&, const char*) {}
void GeckoEngine::load_uri(const char*) {}
void GeckoEngine::stop_loading() {}

void GeckoEngine::find_text(const std::string& text) { last_search_ = text; }
void GeckoEngine::find_next() {}
void GeckoEngine::find_previous() {}
void GeckoEngine::clear_find() { last_search_.clear(); }
void GeckoEngine::apply_config(const Config&) {}
void GeckoEngine::setup_downloads(DownloadManager&) {}
void GeckoEngine::clear_site_data() {}
