#include "tab.hpp"

#include "browser_engine.hpp"
#include "browser_window.hpp"
#include "config.hpp"
#include "download_manager.hpp"
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
: window_(window),
  engine_(std::make_unique<WebKitEngine>(profile, window.config())) {
    BrowserEngine::Callbacks callbacks;
    callbacks.uri_changed = [this](const char* uri) {
        window_.tab_uri_changed(this, uri);
    };
    callbacks.title_changed = [this](const char* title) {
        set_title(title);
    };
    callbacks.load_finished = [this]() {
        window_.tab_load_finished(this);
    };
    engine_->set_callbacks(std::move(callbacks));

    if (web_view() != nullptr) {
        gtk_widget_set_hexpand(web_view(), TRUE);
        gtk_widget_set_vexpand(web_view(), TRUE);
    }

    live_tabs().push_back(this);
    apply_config();

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

    if (window_.download_manager() != nullptr) {
        engine_->setup_downloads(*window_.download_manager());
    }
}

Tab::~Tab() {
    auto& tabs = live_tabs();
    tabs.erase(std::remove(tabs.begin(), tabs.end(), this), tabs.end());
    engine_.reset();
    tab_control_ = nullptr;
    select_button_ = nullptr;
    title_label_ = nullptr;
}

GtkWidget* Tab::web_view() const {
    return engine_ ? engine_->widget() : nullptr;
}

gpointer Tab::native_view() const {
    return engine_ ? engine_->native_handle() : nullptr;
}

GtkWidget* Tab::tab_control() const { return tab_control_; }

void Tab::apply_config() {
    if (engine_) engine_->apply_config(window_.config());

    if (window_.download_manager() != nullptr) {
        window_.download_manager()->set_download_directory(
            window_.config().download_directory());
        window_.download_manager()->set_ask_download_location(
            window_.config().ask_download_location());
        window_.download_manager()->set_parent_window(
            GTK_WINDOW(window_.widget()));
    }
}

void Tab::prepare_for_close() {
    if (engine_ == nullptr) return;
    engine_->stop_loading();
    engine_->clear_find();
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
    if (engine_) engine_->load_html(document, "extart://home/");
    g_bytes_unref(bytes);
}

void Tab::load_uri(const char* uri) {
    if (engine_) engine_->load_uri(uri);
}

void Tab::set_title(const char* title) {
    gtk_label_set_text(GTK_LABEL(title_label_), title && *title ? title : "New tab");
}

void Tab::set_active(bool active) {
    active_ = active;
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

void Tab::find_text(const std::string& text) {
    if (engine_) engine_->find_text(text);
}

void Tab::find_next() {
    if (engine_) engine_->find_next();
}

void Tab::find_previous() {
    if (engine_) engine_->find_previous();
}

void Tab::clear_find() {
    if (engine_) engine_->clear_find();
}
