#include "tab.hpp"

#include "browser_window.hpp"
#include "profile.hpp"

#include <string>

Tab::Tab(BrowserWindow& window, Profile& profile)
    : window_(window) {
    web_view_ = GTK_WIDGET(g_object_new(
        WEBKIT_TYPE_WEB_VIEW,
        "network-session", profile.network_session(),
        nullptr));
    gtk_widget_set_hexpand(web_view_, TRUE);
    gtk_widget_set_vexpand(web_view_, TRUE);
    g_signal_connect(web_view_, "load-changed", G_CALLBACK(on_load_changed), this);

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
}

GtkWidget* Tab::web_view() const {
    return web_view_;
}

GtkWidget* Tab::tab_control() const {
    return tab_control_;
}

WebKitWebView* Tab::view() const {
    return WEBKIT_WEB_VIEW(web_view_);
}

void Tab::load_home() {
    GError* error = nullptr;
    GBytes* bytes = g_resources_lookup_data(
        "/cl/extart/home.html", G_RESOURCE_LOOKUP_FLAGS_NONE, &error);
    if (bytes == nullptr) {
        g_warning("Unable to load the new-tab resource: %s", error->message);
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
    webkit_web_view_load_uri(view(), uri);
}

void Tab::set_title(const char* title) {
    gtk_label_set_text(GTK_LABEL(title_label_), title && *title ? title : "New tab");
}

void Tab::set_active(bool active) {
    if (active) {
        gtk_widget_add_css_class(select_button_, "active-tab");
    } else {
        gtk_widget_remove_css_class(select_button_, "active-tab");
    }
}

void Tab::on_tab_selected(GtkButton*, gpointer user_data) {
    static_cast<Tab*>(user_data)->window_.select_tab(static_cast<Tab*>(user_data));
}

void Tab::on_close_clicked(GtkButton*, gpointer user_data) {
    static_cast<Tab*>(user_data)->window_.close_tab(static_cast<Tab*>(user_data));
}

void Tab::on_load_changed(WebKitWebView* view, WebKitLoadEvent event, gpointer user_data) {
    auto* tab = static_cast<Tab*>(user_data);
    if (event == WEBKIT_LOAD_COMMITTED) {
        tab->window_.tab_uri_changed(tab, webkit_web_view_get_uri(view));
    } else if (event == WEBKIT_LOAD_FINISHED) {
        tab->set_title(webkit_web_view_get_title(view));
    }
}
