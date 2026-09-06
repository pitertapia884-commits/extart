#include "tab.hpp"

#include "download_manager.hpp"
#include "browser_window.hpp"
#include "config.hpp"
#include "profile.hpp"

#include <string>

Tab::Tab(BrowserWindow& window, Profile& profile)
: window_(window) {
    web_view_ = GTK_WIDGET(g_object_new(
        WEBKIT_TYPE_WEB_VIEW,
        "network-session", profile.network_session(),
        nullptr
    ));

    gtk_widget_set_hexpand(web_view_, TRUE);
    gtk_widget_set_vexpand(web_view_, TRUE);

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
    }

    g_signal_connect(
        web_view_,
        "load-changed",
        G_CALLBACK(on_load_changed),
        this
    );

    if (window_.download_manager() != nullptr) {
        window_.download_manager()->setup_for_web_view(view());
    }

    tab_control_ = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL,
        2
    );

    select_button_ = gtk_button_new();
    title_label_ = gtk_label_new("New tab");

    gtk_button_set_child(
        GTK_BUTTON(select_button_),
        title_label_
    );

    gtk_widget_add_css_class(
        select_button_,
        "tab-button"
    );

    GtkWidget* close_button =
        gtk_button_new_with_label("×");

    gtk_widget_add_css_class(
        close_button,
        "tab-close"
    );

    gtk_box_append(
        GTK_BOX(tab_control_),
        select_button_
    );

    gtk_box_append(
        GTK_BOX(tab_control_),
        close_button
    );

    g_signal_connect(
        select_button_,
        "clicked",
        G_CALLBACK(on_tab_selected),
        this
    );

    g_signal_connect(
        close_button,
        "clicked",
        G_CALLBACK(on_close_clicked),
        this
    );

    find_controller_ =
        webkit_web_view_get_find_controller(view());
}

Tab::~Tab() {
    find_controller_ = nullptr;
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
        "/cl/extart/home.html",
        G_RESOURCE_LOOKUP_FLAGS_NONE,
        &error
    );

    if (bytes == nullptr) {
        g_warning(
            "Unable to load the new-tab resource: %s",
            error ? error->message : "unknown error"
        );

        g_clear_error(&error);
        return;
    }

    gsize size = 0;

    const gchar* html =
        static_cast<const gchar*>(
            g_bytes_get_data(bytes, &size)
        );

    std::string document(html, size);

    webkit_web_view_load_html(
        view(),
        document.c_str(),
        "extart://home/"
    );

    g_bytes_unref(bytes);
}

void Tab::load_uri(const char* uri) {
    if (uri == nullptr || *uri == '\0') {
        return;
    }

    webkit_web_view_load_uri(
        view(),
        uri
    );
}

void Tab::set_title(const char* title) {
    gtk_label_set_text(
        GTK_LABEL(title_label_),
        title && *title
            ? title
            : "New tab"
    );
}

void Tab::set_active(bool active) {
    if (active) {
        gtk_widget_add_css_class(
            select_button_,
            "active-tab"
        );
    } else {
        gtk_widget_remove_css_class(
            select_button_,
            "active-tab"
        );
    }
}

void Tab::on_tab_selected(
    GtkButton*,
    gpointer user_data
) {
    auto* tab =
        static_cast<Tab*>(user_data);

    tab->window_.select_tab(tab);
}

void Tab::on_close_clicked(
    GtkButton*,
    gpointer user_data
) {
    auto* tab =
        static_cast<Tab*>(user_data);

    tab->window_.close_tab(tab);
}

void Tab::on_load_changed(
    WebKitWebView* view,
    WebKitLoadEvent event,
    gpointer user_data
) {
    auto* tab =
        static_cast<Tab*>(user_data);

    if (event == WEBKIT_LOAD_COMMITTED) {
        tab->window_.tab_uri_changed(
            tab,
            webkit_web_view_get_uri(view)
        );
    }

    if (event == WEBKIT_LOAD_FINISHED) {
        tab->set_title(
            webkit_web_view_get_title(view)
        );

        tab->window_.tab_load_finished(tab);
    }
}

void Tab::find_text(const std::string& text) {
    if (!find_controller_) {
        return;
    }

    last_search_ = text;

    if (text.empty()) {
        webkit_find_controller_search_finish(
            find_controller_
        );
    } else {
        webkit_find_controller_search(
            find_controller_,
            text.c_str(),
            WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE,
            G_MAXUINT
        );
    }
}

void Tab::find_next() {
    if (!find_controller_ || last_search_.empty()) {
        return;
    }

    webkit_find_controller_search_next(
        find_controller_
    );
}

void Tab::find_previous() {
    if (!find_controller_ || last_search_.empty()) {
        return;
    }

    webkit_find_controller_search_previous(
        find_controller_
    );
}

void Tab::clear_find() {
    if (!find_controller_) {
        return;
    }

    webkit_find_controller_search_finish(
        find_controller_
    );

    last_search_.clear();
}
