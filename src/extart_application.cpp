#include "extart_application.hpp"

#include "browser_window.hpp"
#include "session_manager.hpp"
#include "settings_window.hpp"

#include <algorithm>

ExtartApplication::ExtartApplication() {
    config_.load();
    gtk_application_ = gtk_application_new(
        "cl.extart.browser", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gtk_application_, "activate", G_CALLBACK(on_activate), this);

    GSimpleAction* new_window = g_simple_action_new("new-window", nullptr);
    g_signal_connect(new_window, "activate", G_CALLBACK(on_new_window), this);
    g_action_map_add_action(G_ACTION_MAP(gtk_application_), G_ACTION(new_window));
    g_object_unref(new_window);

    GSimpleAction* settings = g_simple_action_new("settings", nullptr);
    g_signal_connect(settings, "activate", G_CALLBACK(on_settings), this);
    g_action_map_add_action(G_ACTION_MAP(gtk_application_), G_ACTION(settings));
    g_object_unref(settings);

    static const char* const new_window_accelerators[] = {"<Control>n", nullptr};
    gtk_application_set_accels_for_action(
        gtk_application_, "app.new-window", new_window_accelerators);

    static const char* const settings_accelerators[] = {"<Control>comma", nullptr};
    gtk_application_set_accels_for_action(
        gtk_application_, "app.settings", settings_accelerators);
}

ExtartApplication::~ExtartApplication() {
    settings_window_.reset();
    windows_.clear();
    g_clear_object(&gtk_application_);
}

int ExtartApplication::run(int argc, char* argv[]) {
    return g_application_run(G_APPLICATION(gtk_application_), argc, argv);
}

void ExtartApplication::present_or_create_window() {
    if (!windows_.empty()) {
        gtk_window_present(GTK_WINDOW(windows_.front()->widget()));
        return;
    }
    create_window();
}

void ExtartApplication::create_window() {
    const bool first_window = windows_.empty();

    load_css();
    auto window = std::make_unique<BrowserWindow>(*this, gtk_application_, profile_, config_);
    GtkWidget* widget = window->widget();
    g_signal_connect(
        widget, "close-request", G_CALLBACK(on_window_close_request), this);
    g_signal_connect(widget, "destroy", G_CALLBACK(on_window_destroyed), this);
    windows_.push_back(std::move(window));

    if (first_window && config_.restore_session()) {
        session_restore(widget);
    }
}

void ExtartApplication::load_css() {
    if (css_loaded_) return;

    GdkDisplay* display = gdk_display_get_default();
    if (display == nullptr) return;

    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/cl/extart/style.css");
    gtk_style_context_add_provider_for_display(
        display,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
    css_loaded_ = true;
}

void ExtartApplication::remove_window(GtkWidget* widget) {
    auto it = std::find_if(
        windows_.begin(), windows_.end(),
        [widget](const std::unique_ptr<BrowserWindow>& window) {
            return window->widget() == widget;
        });
    if (it != windows_.end()) windows_.erase(it);
}

void ExtartApplication::show_settings() {
    if (!settings_window_) {
        settings_window_ = std::make_unique<SettingsWindow>(
            gtk_application_, config_, profile_);
    }
    settings_window_->present();
}

void ExtartApplication::on_activate(GtkApplication*, gpointer user_data) {
    static_cast<ExtartApplication*>(user_data)->present_or_create_window();
}

void ExtartApplication::on_new_window(GSimpleAction*, GVariant*, gpointer user_data) {
    static_cast<ExtartApplication*>(user_data)->create_window();
}

void ExtartApplication::on_settings(GSimpleAction*, GVariant*, gpointer user_data) {
    static_cast<ExtartApplication*>(user_data)->show_settings();
}

gboolean ExtartApplication::on_window_close_request(GtkWindow* window, gpointer user_data) {
    auto* application = static_cast<ExtartApplication*>(user_data);
    if (application == nullptr || window == nullptr) return FALSE;

    for (const auto& browser_window : application->windows_) {
        if (browser_window->widget() == GTK_WIDGET(window)) {
            if (application->config_.restore_session()) {
                session_save(browser_window->widget());
            }
            browser_window->prepare_for_shutdown();
            break;
        }
    }
    return FALSE;
}

void ExtartApplication::on_window_destroyed(GtkWidget* widget, gpointer user_data) {
    static_cast<ExtartApplication*>(user_data)->remove_window(widget);
}
