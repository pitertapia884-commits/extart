#pragma once

#include <gtk/gtk.h>

#include <memory>
#include <vector>

class Config;
class ExtartApplication;
class Profile;
class Tab;

class BrowserWindow {
public:
    BrowserWindow(ExtartApplication& application, GtkApplication* gtk_application,
                  Profile& profile, Config& config);
    ~BrowserWindow();

    GtkWidget* widget() const;
    void select_tab(Tab* tab);
    void close_tab(Tab* tab);
    void tab_uri_changed(Tab* tab, const char* uri);

private:
    Tab* active_tab() const;
    Tab& open_tab();
    void navigate_from_entry();

    static void on_address_activate(GtkEntry* entry, gpointer user_data);
    static void on_back_clicked(GtkButton* button, gpointer user_data);
    static void on_forward_clicked(GtkButton* button, gpointer user_data);
    static void on_reload_clicked(GtkButton* button, gpointer user_data);
    static void on_home_clicked(GtkButton* button, gpointer user_data);
    static void on_new_tab_clicked(GtkButton* button, gpointer user_data);

    ExtartApplication& application_;
    Profile& profile_;
    Config& config_;
    GtkWidget* window_ = nullptr;
    GtkWidget* tab_bar_ = nullptr;
    GtkWidget* content_stack_ = nullptr;
    GtkWidget* url_bar_ = nullptr;
    std::vector<std::unique_ptr<Tab>> tabs_;
    Tab* active_tab_ = nullptr;
};
