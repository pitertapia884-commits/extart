#pragma once

#include <gtk/gtk.h>

class Config;
class Profile;

class SettingsWindow {
public:
    SettingsWindow(GtkApplication* application, Config& config, Profile& profile);
    ~SettingsWindow();

    SettingsWindow(const SettingsWindow&) = delete;
    SettingsWindow& operator=(const SettingsWindow&) = delete;

    void present();

private:
    static void on_save_clicked(GtkButton* button, gpointer user_data);
    static void on_clear_data_clicked(GtkButton* button, gpointer user_data);
    static gboolean on_close_request(GtkWindow* window, gpointer user_data);

    void save();
    void clear_site_data();
    void create_ui();

    GtkApplication* application_ = nullptr;
    Config& config_;
    Profile& profile_;

    GtkWidget* window_ = nullptr;
    GtkWidget* search_engine_entry_ = nullptr;
    GtkWidget* download_directory_entry_ = nullptr;
    GtkWidget* homepage_entry_ = nullptr;
    GtkWidget* ask_download_location_ = nullptr;
    GtkWidget* restore_session_ = nullptr;
    GtkWidget* javascript_ = nullptr;
    GtkWidget* images_ = nullptr;
    GtkWidget* sound_ = nullptr;
    GtkWidget* popups_ = nullptr;
};
