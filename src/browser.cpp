#include "browser.h"
#include <string>

Browser* Browser::_instance = nullptr;

Browser::Browser(GtkApplication* app) {
    _instance = this;
    build_ui(app);
}

Browser* Browser::instance() {
    return _instance;
}

void Browser::build_ui(GtkApplication* app) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "EXTART");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget* navbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_start(navbar, 6);
    gtk_widget_set_margin_end(navbar, 6);
    gtk_widget_set_margin_top(navbar, 6);
    gtk_widget_set_margin_bottom(navbar, 6);
    gtk_widget_add_css_class(navbar, "navbar");

    GtkWidget* btn_back    = gtk_button_new_with_label("←");
    GtkWidget* btn_forward = gtk_button_new_with_label("→");
    GtkWidget* btn_reload  = gtk_button_new_with_label("↺");

    url_bar = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(url_bar), "https://duckduckgo.com");
    gtk_widget_set_hexpand(url_bar, TRUE);

    g_signal_connect(url_bar,     "activate", G_CALLBACK(on_navigate),    NULL);
    g_signal_connect(btn_back,    "clicked",  G_CALLBACK(on_back),        NULL);
    g_signal_connect(btn_forward, "clicked",  G_CALLBACK(on_forward),     NULL);
    g_signal_connect(btn_reload,  "clicked",  G_CALLBACK(on_reload),      NULL);

    gtk_box_append(GTK_BOX(navbar), btn_back);
    gtk_box_append(GTK_BOX(navbar), btn_forward);
    gtk_box_append(GTK_BOX(navbar), btn_reload);
    gtk_box_append(GTK_BOX(navbar), url_bar);

    webview = webkit_web_view_new();
    gtk_widget_set_vexpand(webview, TRUE);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), "file:///home/arch/extart/home.html");

    g_signal_connect(webview, "load-changed", G_CALLBACK(on_load_changed), NULL);

    gtk_box_append(GTK_BOX(vbox), navbar);
    gtk_box_append(GTK_BOX(vbox), webview);

    gtk_window_set_child(GTK_WINDOW(window), vbox);

    load_css();

    gtk_window_present(GTK_WINDOW(window));
}

void Browser::load_css() {
    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css, "../style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

void Browser::on_navigate(GtkWidget* widget, gpointer user_data) {
    Browser* b = Browser::instance();
    const char* url = gtk_editable_get_text(GTK_EDITABLE(b->url_bar));
    std::string uri(url);

    if (uri.find("http://") != 0 && uri.find("https://") != 0) {
        uri = "https://" + uri;
    }

    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(b->webview), uri.c_str());
}

void Browser::on_back(GtkWidget* widget, gpointer user_data) {
    webkit_web_view_go_back(WEBKIT_WEB_VIEW(Browser::instance()->webview));
}

void Browser::on_forward(GtkWidget* widget, gpointer user_data) {
    webkit_web_view_go_forward(WEBKIT_WEB_VIEW(Browser::instance()->webview));
}

void Browser::on_reload(GtkWidget* widget, gpointer user_data) {
    webkit_web_view_reload(WEBKIT_WEB_VIEW(Browser::instance()->webview));
}

void Browser::on_load_changed(WebKitWebView* wv, WebKitLoadEvent event, gpointer user_data) {
    if (event == WEBKIT_LOAD_COMMITTED) {
        const char* uri = webkit_web_view_get_uri(wv);
        if (uri) {
            gtk_editable_set_text(GTK_EDITABLE(Browser::instance()->url_bar), uri);
        }
    }
}