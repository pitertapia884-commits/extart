#pragma once

#include "config.hpp"
#include "profile.hpp"

#include <gtk/gtk.h>

#include <memory>
#include <vector>

class BrowserWindow;

class ExtartApplication {
public:
    ExtartApplication();
    ~ExtartApplication();

    ExtartApplication(const ExtartApplication&) = delete;
    ExtartApplication& operator=(const ExtartApplication&) = delete;

    int run(int argc, char* argv[]);

private:
    void create_window();
    void load_css();
    void present_or_create_window();
    void remove_window(GtkWidget* widget);

    static void on_activate(GtkApplication* application, gpointer user_data);
    static void on_new_window(GSimpleAction* action, GVariant* parameter, gpointer user_data);
    static void on_window_destroyed(GtkWidget* widget, gpointer user_data);

    GtkApplication* gtk_application_ = nullptr;
    Config config_;
    Profile profile_;
    std::vector<std::unique_ptr<BrowserWindow>> windows_;
    bool css_loaded_ = false;
};
