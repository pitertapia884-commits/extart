#include "settings_window.hpp"

#include "config.hpp"
#include "profile.hpp"
#include "tab.hpp"

SettingsWindow::SettingsWindow(GtkApplication* application, Config& config, Profile& profile)
    : application_(application), config_(config), profile_(profile) {
    create_ui();
}

SettingsWindow::~SettingsWindow() {
    if (window_ != nullptr) {
        gtk_window_destroy(GTK_WINDOW(window_));
        window_ = nullptr;
    }
}

void SettingsWindow::create_ui() {
    window_ = gtk_application_window_new(application_);
    gtk_window_set_title(GTK_WINDOW(window_), "EXTART — Configuración");
    gtk_window_set_default_size(GTK_WINDOW(window_), 560, 620);

    GtkWidget* root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(root, 18);
    gtk_widget_set_margin_bottom(root, 18);
    gtk_widget_set_margin_start(root, 18);
    gtk_widget_set_margin_end(root, 18);

    GtkWidget* title = gtk_label_new("Configuración");
    gtk_widget_add_css_class(title, "title-2");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_box_append(GTK_BOX(root), title);

    GtkWidget* general = gtk_frame_new("General");
    GtkWidget* general_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(general_box, 10);
    gtk_widget_set_margin_bottom(general_box, 10);
    gtk_widget_set_margin_start(general_box, 10);
    gtk_widget_set_margin_end(general_box, 10);

    homepage_entry_ = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(homepage_entry_), config_.homepage().c_str());
    gtk_box_append(GTK_BOX(general_box), gtk_label_new("Página de inicio"));
    gtk_box_append(GTK_BOX(general_box), homepage_entry_);

    search_engine_entry_ = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(search_engine_entry_), config_.search_engine().c_str());
    gtk_box_append(GTK_BOX(general_box), gtk_label_new("Motor de búsqueda"));
    gtk_box_append(GTK_BOX(general_box), search_engine_entry_);

    restore_session_ = gtk_check_button_new_with_label("Restaurar sesión al iniciar");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(restore_session_), config_.restore_session());
    gtk_box_append(GTK_BOX(general_box), restore_session_);

    gtk_frame_set_child(GTK_FRAME(general), general_box);
    gtk_box_append(GTK_BOX(root), general);

    GtkWidget* downloads = gtk_frame_new("Descargas");
    GtkWidget* downloads_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(downloads_box, 10);
    gtk_widget_set_margin_bottom(downloads_box, 10);
    gtk_widget_set_margin_start(downloads_box, 10);
    gtk_widget_set_margin_end(downloads_box, 10);

    download_directory_entry_ = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(download_directory_entry_), config_.download_directory().c_str());
    gtk_box_append(GTK_BOX(downloads_box), gtk_label_new("Carpeta de descargas"));
    gtk_box_append(GTK_BOX(downloads_box), download_directory_entry_);

    ask_download_location_ = gtk_check_button_new_with_label("Preguntar dónde guardar cada descarga");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(ask_download_location_), config_.ask_download_location());
    gtk_box_append(GTK_BOX(downloads_box), ask_download_location_);

    gtk_frame_set_child(GTK_FRAME(downloads), downloads_box);
    gtk_box_append(GTK_BOX(root), downloads);

    GtkWidget* web = gtk_frame_new("Contenido web");
    GtkWidget* web_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(web_box, 10);
    gtk_widget_set_margin_bottom(web_box, 10);
    gtk_widget_set_margin_start(web_box, 10);
    gtk_widget_set_margin_end(web_box, 10);

    javascript_ = gtk_check_button_new_with_label("JavaScript");
    images_ = gtk_check_button_new_with_label("Imágenes");
    sound_ = gtk_check_button_new_with_label("Sonido");
    popups_ = gtk_check_button_new_with_label("Ventanas emergentes");

    gtk_check_button_set_active(GTK_CHECK_BUTTON(javascript_), config_.javascript_enabled());
    gtk_check_button_set_active(GTK_CHECK_BUTTON(images_), config_.images_enabled());
    gtk_check_button_set_active(GTK_CHECK_BUTTON(sound_), config_.sound_enabled());
    gtk_check_button_set_active(GTK_CHECK_BUTTON(popups_), config_.popups_enabled());

    gtk_box_append(GTK_BOX(web_box), javascript_);
    gtk_box_append(GTK_BOX(web_box), images_);
    gtk_box_append(GTK_BOX(web_box), sound_);
    gtk_box_append(GTK_BOX(web_box), popups_);

    gtk_frame_set_child(GTK_FRAME(web), web_box);
    gtk_box_append(GTK_BOX(root), web);

    GtkWidget* buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(buttons, GTK_ALIGN_END);

    GtkWidget* clear_button = gtk_button_new_with_label("Limpiar datos del sitio");
    g_signal_connect(clear_button, "clicked", G_CALLBACK(on_clear_data_clicked), this);
    gtk_box_append(GTK_BOX(buttons), clear_button);

    GtkWidget* save_button = gtk_button_new_with_label("Guardar");
    gtk_widget_add_css_class(save_button, "suggested-action");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), this);
    gtk_box_append(GTK_BOX(buttons), save_button);

    gtk_box_append(GTK_BOX(root), buttons);
    gtk_window_set_child(GTK_WINDOW(window_), root);
    g_signal_connect(window_, "close-request", G_CALLBACK(on_close_request), this);
}

void SettingsWindow::present() {
    if (window_ == nullptr) create_ui();
    gtk_window_present(GTK_WINDOW(window_));
}

void SettingsWindow::save() {
    config_.set_homepage(gtk_editable_get_text(GTK_EDITABLE(homepage_entry_)));
    config_.set_search_engine(gtk_editable_get_text(GTK_EDITABLE(search_engine_entry_)));
    config_.set_download_directory(gtk_editable_get_text(GTK_EDITABLE(download_directory_entry_)));
    config_.set_restore_session(gtk_check_button_get_active(GTK_CHECK_BUTTON(restore_session_)));
    config_.set_ask_download_location(gtk_check_button_get_active(GTK_CHECK_BUTTON(ask_download_location_)));
    config_.set_javascript_enabled(gtk_check_button_get_active(GTK_CHECK_BUTTON(javascript_)));
    config_.set_images_enabled(gtk_check_button_get_active(GTK_CHECK_BUTTON(images_)));
    config_.set_sound_enabled(gtk_check_button_get_active(GTK_CHECK_BUTTON(sound_)));
    config_.set_popups_enabled(gtk_check_button_get_active(GTK_CHECK_BUTTON(popups_)));
    config_.save();

    // Los WebViews ya abiertos reciben la nueva configuración inmediatamente.
    Tab::apply_config_to_all_tabs();
}

void SettingsWindow::clear_site_data() {
    WebKitWebsiteDataManager* manager =
        webkit_network_session_get_website_data_manager(profile_.network_session());

    if (manager == nullptr) return;

    webkit_website_data_manager_clear(
        manager,
        WEBKIT_WEBSITE_DATA_ALL,
        0,
        nullptr,
        nullptr,
        nullptr
    );
}

void SettingsWindow::on_save_clicked(GtkButton*, gpointer user_data) {
    auto* self = static_cast<SettingsWindow*>(user_data);
    if (self != nullptr) self->save();
}

void SettingsWindow::on_clear_data_clicked(GtkButton*, gpointer user_data) {
    auto* self = static_cast<SettingsWindow*>(user_data);
    if (self != nullptr) self->clear_site_data();
}

gboolean SettingsWindow::on_close_request(GtkWindow*, gpointer user_data) {
    auto* self = static_cast<SettingsWindow*>(user_data);
    if (self != nullptr) self->window_ = nullptr;
    return FALSE;
}
