#pragma once

#include <gtk/gtk.h>
#include <webkit/webkit.h>

class Browser {
public:
    Browser(GtkApplication* app);

    static Browser* instance();

private:
    static Browser* _instance;

    GtkWidget* window;
    GtkWidget* webview;
    GtkWidget* url_bar;

    void build_ui(GtkApplication* app);
    void load_css();

    // Callbacks estáticos para GTK
    static void on_navigate(GtkWidget* widget, gpointer user_data);
    static void on_back(GtkWidget* widget, gpointer user_data);
    static void on_forward(GtkWidget* widget, gpointer user_data);
    static void on_reload(GtkWidget* widget, gpointer user_data);
    static void on_load_changed(WebKitWebView* wv, WebKitLoadEvent event, gpointer user_data);
    static void on_activate(GtkApplication* app, gpointer user_data);
};