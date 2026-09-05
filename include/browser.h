#pragma once

#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <string>

class Browser {
public:
    Browser(GtkApplication* app);
    static Browser* instance();

private:
    static Browser* _instance;
    bool switching_tab = false;

    GtkWidget* window;
    GtkWidget* notebook;
    GtkWidget* url_bar;

    void build_ui(GtkApplication* app);
    void load_css();

    void new_tab(const std::string& url = "file:///home/arch/extart/home.html");
    WebKitWebView* get_current_webview();

    static void on_navigate(GtkWidget* widget, gpointer user_data);
    static void on_back(GtkWidget* widget, gpointer user_data);
    static void on_forward(GtkWidget* widget, gpointer user_data);
    static void on_reload(GtkWidget* widget, gpointer user_data);
    static void on_home(GtkWidget* widget, gpointer user_data);
    static void on_new_tab(GtkWidget* widget, gpointer user_data);
    static void on_load_changed(WebKitWebView* wv, WebKitLoadEvent event, gpointer user_data);
    static void on_switch_page(GtkNotebook* notebook, GtkWidget* page, guint page_num, gpointer user_data);
    static void on_close_tab(GtkWidget* wv, GtkWidget* button);
    static void on_settings(GtkWidget* widget, gpointer user_data);
    static void on_downloads(GtkWidget* widget, gpointer user_data);
    static gboolean on_decide_policy(WebKitWebView* wv, WebKitPolicyDecision* decision, WebKitPolicyDecisionType type, gpointer user_data);
};