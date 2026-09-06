#pragma once

#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <string>

class BrowserWindow;
class Config;
class Profile;

class Tab {
public:
    Tab(BrowserWindow& window, Profile& profile, Config& config);
    ~Tab();

    GtkWidget* web_view() const;
    GtkWidget* tab_control() const;
    WebKitWebView* view() const;

    void load_home();
    void load_uri(const char* uri);
    void set_title(const char* title);
    void set_active(bool active);
    
    // Búsqueda en página
    void find_text(const std::string& text);
    void find_next();
    void find_previous();
    void clear_find();

private:
    static void on_tab_selected(GtkButton* button, gpointer user_data);
    static void on_close_clicked(GtkButton* button, gpointer user_data);
    static void on_load_changed(WebKitWebView* view, WebKitLoadEvent event, gpointer user_data);

    BrowserWindow& window_;
    GtkWidget* web_view_ = nullptr;
    GtkWidget* tab_control_ = nullptr;
    GtkWidget* select_button_ = nullptr;
    GtkWidget* title_label_ = nullptr;
    WebKitFindController* find_controller_ = nullptr;
    std::string last_search_;
};
