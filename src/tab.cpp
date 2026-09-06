#include "tab.hpp"

#include "download_manager.hpp"
#include "browser_window.hpp"
#include "config.hpp"
#include "profile.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace {
std::vector<Tab*>& live_tabs() {
    static std::vector<Tab*> tabs;
    return tabs;
}
}

Tab::Tab(BrowserWindow& window, Profile& profile)
: window_(window) {
    web_view_ = GTK_WIDGET(g_object_new(
        WEBKIT_TYPE_WEB_VIEW,
        "network-session", profile.network_session(),
        nullptr
    ));

    gtk_widget_set_hexpand(web_view_, TRUE);
    gtk_widget_set_vexpand(web_view_, TRUE);

    live_tabs().push_back(this);
    apply_config();

    g_signal_connect(web_view_, "load-changed", G_CALLBACK(on_load_changed), this);

    if (window_.download_manager() != nullptr) {
        window_.download_manager()->setup_for_web_view(view());
    }

    tab_control_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    select_button_ = gtk_button_new();
    title_label_ = gtk_label_new("New tab");
    gtk_button_set_child(GTK_BUTTON(select_button_), title_label_);
    gtk_widget_add_css_class(select_button_, "tab-button");

    GtkWidget* close_button = gtk_button_new_with_label("×");
    gtk_widget_add_css_class(close_button, "tab-close");
    gtk_box_append(GTK_BOX(tab_control_), select_button_);
    gtk_box_append(GTK_BOX(tab_control_), close_button);

    g_signal_connect(select_button_, "clicked", G_CALLBACK(on_tab_selected), this);
    g_signal_connect(close_button, "clicked", G_CALLBACK(on_close_clicked), this);
    find_controller_ = webkit_web_view_get_find_controller(view());
}

Tab::~Tab() {
    // The WebView is removed from the GTK hierarchy by BrowserWindow before
    // this object is destroyed. Do not call WebKit APIs here: the widget may
    // already have been finalized by GTK at this point.
    auto& tabs = live_tabs();
    tabs.erase(std::remove(tabs.begin(), tabs.end(), this), tabs.end());
    find_controller_ = nullptr;
    web_view_ = nullptr;
    tab_control_ = nullptr;
    select_button_ = nullptr;
    title_label_ = nullptr;
}

GtkWidget* Tab::web_view() const { return web_view_; }
GtkWidget* Tab::tab_control() const { return tab_control_; }
WebKitWebView* Tab::view() const { return WEBKIT_WEB_VIEW(web_view_); }

void Tab::apply_config() {
    const Config& config = window_.config();
    WebKitSettings* settings = webkit_web_view_get_settings(view());

    if (settings != nullptr) {
        webkit_settings_set_enable_javascript(settings, config.javascript_enabled());
        webkit_settings_set_auto_load_images(settings, config.images_enabled());
        webkit_settings_set_enable_media(settings, config.sound_enabled());
        webkit_settings_set_enable_webaudio(settings, config.sound_enabled());
        webkit_settings_set_javascript_can_open_windows_automatically(
            settings,
            config.popups_enabled()
        );

        // Keep WebKit's normal page cache enabled. Disabling it made
        // navigation noticeably slower and is not an appropriate substitute
        // for fixing actual memory retention.
        webkit_settings_set_enable_page_cache(settings, TRUE);

        // Keep a modern browser-compatible identity while explicitly branding
        // the browser as EXTART. ASCII is intentional for HTTP interoperability.
        webkit_settings_set_user_agent(
            settings,
            "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
            "(KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36 "
            "EXTART/0.4 (Bread)"
        );
    }

    if (window_.download_manager() != nullptr) {
        window_.download_manager()->set_download_directory(config.download_directory());
        window_.download_manager()->set_ask_download_location(config.ask_download_location());
        window_.download_manager()->set_parent_window(GTK_WINDOW(window_.widget()));
    }
}

void Tab::prepare_for_close() {
    if (web_view_ == nullptr) return;

    // Must be called before BrowserWindow removes the WebView from GTK.
    webkit_web_view_stop_loading(view());

    if (find_controller_ != nullptr) {
        webkit_find_controller_search_finish(find_controller_);
    }

    last_search_.clear();
}

void Tab::apply_config_to_all_tabs() {
    for (Tab* tab : live_tabs()) {
        if (tab != nullptr) tab->apply_config();
    }
}

void Tab::load_home() {
    GError* error = nullptr;
    GBytes* bytes = g_resources_lookup_data(
        "/cl/extart/home.html", G_RESOURCE_LOOKUP_FLAGS_NONE, &error);

    if (bytes == nullptr) {
        g_warning("Unable to load the new-tab resource: %s",
                  error ? error->message : "unknown error");
        g_clear_error(&error);
        return;
    }

    gsize size = 0;
    const gchar* html = static_cast<const gchar*>(g_bytes_get_data(bytes, &size));
    std::string document(html, size);
    webkit_web_view_load_html(view(), document.c_str(), "extart://home/");
    g_bytes_unref(bytes);
}

void Tab::load_uri(const char* uri) {
    if (uri == nullptr || *uri == '\0') return;
    webkit_web_view_load_uri(view(), uri);
}

void Tab::set_title(const char* title) {
    gtk_label_set_text(GTK_LABEL(title_label_), title && *title ? title : "New tab");
}

void Tab::set_active(bool active) {
    if (active) gtk_widget_add_css_class(select_button_, "active-tab");
    else gtk_widget_remove_css_class(select_button_, "active-tab");
}

void Tab::on_tab_selected(GtkButton*, gpointer user_data) {
    auto* tab = static_cast<Tab*>(user_data);
    tab->window_.select_tab(tab);
}

void Tab::on_close_clicked(GtkButton*, gpointer user_data) {
    auto* tab = static_cast<Tab*>(user_data);
    tab->window_.close_tab(tab);
}

void Tab::on_load_changed(WebKitWebView* view, WebKitLoadEvent event, gpointer user_data) {
    auto* tab = static_cast<Tab*>(user_data);
    if (event == WEBKIT_LOAD_COMMITTED) {
        tab->window_.tab_uri_changed(tab, webkit_web_view_get_uri(view));
    }
    if (event == WEBKIT_LOAD_FINISHED) {
        tab->set_title(webkit_web_view_get_title(view));
        tab->window_.tab_load_finished(tab);
    }
}

void Tab::find_text(const std::string& text) {
    if (!find_controller_) return;
    last_search_ = text;
    if (text.empty()) webkit_find_controller_search_finish(find_controller_);
    else webkit_find_controller_search(find_controller_, text.c_str(),
                                       WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE, G_MAXUINT);
}

void Tab::find_next() {
    if (!find_controller_ || last_search_.empty()) return;
    webkit_find_controller_search_next(find_controller_);
}

void Tab::find_previous() {
    if (!find_controller_ || last_search_.empty()) return;
    webkit_find_controller_search_previous(find_controller_);
}

void Tab::clear_find() {
    if (!find_controller_) return;
    webkit_find_controller_search_finish(find_controller_);
    last_search_.clear();
}
