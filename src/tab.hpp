#pragma once

#include <gtk/gtk.h>
#include <webkit/webkit.h>

class BrowserWindow;
class Profile;

class Tab {
public:
    Tab(BrowserWindow& window, Profile& profile);

    GtkWidget* web_view() const;
    GtkWidget* tab_control() const;
    WebKitWebView* view() const;

    void load_home();
    void load_uri(const char* uri);
    void set_title(const char* title);
    void set_active(bool active);

private:
    static void on_tab_selected(GtkButton* button, gpointer user_data);
    static void on_close_clicked(GtkButton* button, gpointer user_data);
    static void on_load_changed(WebKitWebView* view, WebKitLoadEvent event, gpointer user_data);

    BrowserWindow& window_;
    GtkWidget* web_view_ = nullptr;
    GtkWidget* tab_control_ = nullptr;
    GtkWidget* select_button_ = nullptr;
    GtkWidget* title_label_ = nullptr;
};
