#include "downloads_panel.hpp"

#include "download_manager.hpp"

#include <glib.h>
#include <gio/gio.h>
#include <utility>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

// Helper para formatear tamaños
static std::string format_bytes(uint64_t bytes) {
    const char* units[] = {
        "B",
        "KB",
        "MB",
        "GB"
    };

    double size =
        static_cast<double>(bytes);

    int unit = 0;

    while (size >= 1024.0 &&
           unit < 3) {
        size /= 1024.0;
        unit++;
    }

    std::ostringstream oss;

    oss << std::fixed
        << std::setprecision(1)
        << size
        << " "
        << units[unit];

    return oss.str();
}

DownloadsPanel::DownloadsPanel(
    DownloadManager* dm
)
    : dm_(dm) {
}

DownloadsPanel::~DownloadsPanel() = default;

GtkWidget* DownloadsPanel::create_panel() {
    GtkWidget* box =
        gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            8
        );

    gtk_widget_set_margin_start(
        box,
        12
    );

    gtk_widget_set_margin_end(
        box,
        12
    );

    gtk_widget_set_margin_top(
        box,
        12
    );

    gtk_widget_set_margin_bottom(
        box,
        12
    );

    // Header
    GtkWidget* header =
        gtk_box_new(
            GTK_ORIENTATION_HORIZONTAL,
            8
        );

    GtkWidget* title =
        gtk_label_new("Downloads");

    gtk_widget_add_css_class(
        title,
        "heading"
    );

    gtk_label_set_xalign(
        GTK_LABEL(title),
        0.0
    );

    gtk_widget_set_hexpand(
        title,
        TRUE
    );

    GtkWidget* clear_button =
        gtk_button_new_with_label(
            "Clear completed"
        );

    gtk_widget_add_css_class(
        clear_button,
        "flat"
    );

    // TODO: conectar callback cuando
    // exista soporte para limpiar historial.

    gtk_box_append(
        GTK_BOX(header),
        title
    );

    gtk_box_append(
        GTK_BOX(header),
        clear_button
    );

    // Lista scrollable
    GtkWidget* scrolled =
        gtk_scrolled_window_new();

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scrolled),
        GTK_POLICY_NEVER,
        GTK_POLICY_AUTOMATIC
    );

    gtk_widget_set_vexpand(
        scrolled,
        TRUE
    );

    gtk_widget_set_hexpand(
        scrolled,
        TRUE
    );

    gtk_widget_set_size_request(
        scrolled,
        350,
        300
    );

    list_box_ =
        gtk_list_box_new();

    gtk_widget_add_css_class(
        list_box_,
        "navigation-sidebar"
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scrolled),
        list_box_
    );

    // Label para estado vacío
    empty_label_ =
        gtk_label_new(
            "No downloads"
        );

    gtk_widget_add_css_class(
        empty_label_,
        "dim-label"
    );

    gtk_label_set_justify(
        GTK_LABEL(empty_label_),
        GTK_JUSTIFY_CENTER
    );

    // Stack
    GtkWidget* stack =
        gtk_stack_new();

    gtk_stack_add_named(
        GTK_STACK(stack),
        scrolled,
        "list"
    );

    gtk_stack_add_named(
        GTK_STACK(stack),
        empty_label_,
        "empty"
    );

    gtk_stack_set_visible_child_name(
        GTK_STACK(stack),
        "empty"
    );

    gtk_widget_set_vexpand(
        stack,
        TRUE
    );

    gtk_widget_set_hexpand(
        stack,
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        header
    );

    gtk_box_append(
        GTK_BOX(box),
        stack
    );

    populate_list();

    // Estado inicial
    if (!dm_->downloads().empty()) {
        gtk_stack_set_visible_child_name(
            GTK_STACK(stack),
            "list"
        );
    }

    return box;
}

void DownloadsPanel::populate_list() {
    if (!list_box_) {
        return;
    }

    // Limpiar lista
    while (GtkWidget* child =
               gtk_widget_get_first_child(
                   list_box_
               )) {

        gtk_list_box_remove(
            GTK_LIST_BOX(list_box_),
            child
        );
    }

    download_rows_.clear();

    const auto& downloads =
        dm_->downloads();

    if (downloads.empty()) {
        return;
    }

    // Más recientes primero
    for (auto it =
             downloads.rbegin();
         it != downloads.rend();
         ++it) {

        const auto& dl = *it;

        GtkWidget* row =
            gtk_box_new(
                GTK_ORIENTATION_VERTICAL,
                6
            );

        gtk_widget_set_margin_start(
            row,
            8
        );

        gtk_widget_set_margin_end(
            row,
            8
        );

        gtk_widget_set_margin_top(
            row,
            6
        );

        gtk_widget_set_margin_bottom(
            row,
            6
        );

        // Filename
        GtkWidget* filename_label =
            gtk_label_new(
                dl.filename.c_str()
            );

        gtk_label_set_xalign(
            GTK_LABEL(filename_label),
            0.0
        );

        gtk_widget_add_css_class(
            filename_label,
            "title-4"
        );

        gtk_label_set_wrap(
            GTK_LABEL(filename_label),
            TRUE
        );

        gtk_label_set_ellipsize(
            GTK_LABEL(filename_label),
            PANGO_ELLIPSIZE_MIDDLE
        );

        // Estado / progreso
        std::ostringstream status_str;

        if (dl.completed) {

            status_str
                << "✓ Completed · "
                << format_bytes(
                       dl.size_bytes
                   );

        } else if (dl.size_bytes > 0) {

            double progress =
                static_cast<double>(
                    dl.downloaded_bytes
                ) /
                static_cast<double>(
                    dl.size_bytes
                ) *
                100.0;

            status_str
                << std::fixed
                << std::setprecision(0)
                << progress
                << "% · "
                << format_bytes(
                       dl.downloaded_bytes
                   )
                << " / "
                << format_bytes(
                       dl.size_bytes
                   );

        } else {

            status_str
                << "Starting...";
        }

        GtkWidget* status_label =
            gtk_label_new(
                status_str.str().c_str()
            );

        gtk_label_set_xalign(
            GTK_LABEL(status_label),
            0.0
        );

        gtk_widget_add_css_class(
            status_label,
            "dim-label"
        );

        gtk_widget_add_css_class(
            status_label,
            "small-text"
        );

        // Barra de progreso
        GtkWidget* progress_bar =
            nullptr;

        if (!dl.completed &&
            dl.size_bytes > 0) {

            progress_bar =
                gtk_progress_bar_new();

            double fraction =
                static_cast<double>(
                    dl.downloaded_bytes
                ) /
                static_cast<double>(
                    dl.size_bytes
                );

            fraction =
                std::min(
                    fraction,
                    1.0
                );

            gtk_progress_bar_set_fraction(
                GTK_PROGRESS_BAR(
                    progress_bar
                ),
                fraction
            );

            gtk_widget_set_size_request(
                progress_bar,
                -1,
                4
            );
        }

        // Botones de acción
        GtkWidget* button_box =
            gtk_box_new(
                GTK_ORIENTATION_HORIZONTAL,
                4
            );

        if (dl.completed &&
            !dl.path.empty()) {

            GtkWidget* open_button =
                gtk_button_new_with_label(
                    "Open"
                );

            gtk_widget_add_css_class(
                open_button,
                "flat"
            );

            gtk_widget_set_size_request(
                open_button,
                80,
                -1
            );

            g_signal_connect(
                open_button,
                "clicked",
                G_CALLBACK(on_open_clicked),
                this
            );

            g_object_set_data_full(
                G_OBJECT(open_button),
                "path",
                g_strdup(
                    dl.path.c_str()
                ),
                g_free
            );

            GtkWidget* folder_button =
                gtk_button_new_with_label(
                    "Folder"
                );

            gtk_widget_add_css_class(
                folder_button,
                "flat"
            );

            gtk_widget_set_size_request(
                folder_button,
                80,
                -1
            );

            g_signal_connect(
                folder_button,
                "clicked",
                G_CALLBACK(
                    on_open_folder_clicked
                ),
                this
            );

            g_object_set_data_full(
                G_OBJECT(folder_button),
                "path",
                g_strdup(
                    dl.path.c_str()
                ),
                g_free
            );

            gtk_box_append(
                GTK_BOX(button_box),
                open_button
            );

            gtk_box_append(
                GTK_BOX(button_box),
                folder_button
            );
        }

        GtkWidget* remove_button =
            gtk_button_new_with_label(
                "✕"
            );

        gtk_widget_add_css_class(
            remove_button,
            "flat"
        );

        gtk_widget_set_size_request(
            remove_button,
            30,
            -1
        );

        g_signal_connect(
            remove_button,
            "clicked",
            G_CALLBACK(on_remove_clicked),
            this
        );

        g_object_set_data_full(
            G_OBJECT(remove_button),
            "uri",
            g_strdup(
                dl.uri.c_str()
            ),
            g_free
        );

        gtk_box_append(
            GTK_BOX(button_box),
            remove_button
        );

        // Construir fila
        gtk_box_append(
            GTK_BOX(row),
            filename_label
        );

        gtk_box_append(
            GTK_BOX(row),
            status_label
        );

        if (progress_bar) {
            gtk_box_append(
                GTK_BOX(row),
                progress_bar
            );
        }

        gtk_box_append(
            GTK_BOX(row),
            button_box
        );

        // GtkListBoxRow real
        GtkListBoxRow* list_row =
            GTK_LIST_BOX_ROW(
                gtk_list_box_row_new()
            );

        gtk_list_box_row_set_child(
            list_row,
            row
        );

        gtk_list_box_append(
            GTK_LIST_BOX(list_box_),
            GTK_WIDGET(list_row)
        );

        download_rows_.push_back(
            std::make_pair(
                dl,
                GTK_WIDGET(list_row)
            )
        );
    }
}

void DownloadsPanel::on_open_clicked(
    GtkButton* button,
    gpointer user_data
) {
    auto* panel =
        static_cast<DownloadsPanel*>(
            user_data
        );

    if (!panel) {
        return;
    }

    const char* path =
        static_cast<const char*>(
            g_object_get_data(
                G_OBJECT(button),
                "path"
            )
        );

    if (!path) {
        return;
    }

    GFile* file =
        g_file_new_for_path(path);

    GtkUriLauncher* launcher =
        gtk_uri_launcher_new(
            g_file_get_uri(file)
        );

    gtk_uri_launcher_launch(
        launcher,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    g_object_unref(launcher);
    g_object_unref(file);
}

void DownloadsPanel::on_open_folder_clicked(
    GtkButton* button,
    gpointer user_data
) {
    auto* panel =
        static_cast<DownloadsPanel*>(
            user_data
        );

    if (!panel) {
        return;
    }

    const char* path =
        static_cast<const char*>(
            g_object_get_data(
                G_OBJECT(button),
                "path"
            )
        );

    if (!path) {
        return;
    }

    GFile* file =
        g_file_new_for_path(path);

    GFile* parent =
        g_file_get_parent(file);

    if (parent) {

        GtkUriLauncher* launcher =
            gtk_uri_launcher_new(
                g_file_get_uri(parent)
            );

        gtk_uri_launcher_launch(
            launcher,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );

        g_object_unref(launcher);
        g_object_unref(parent);
    }

    g_object_unref(file);
}

void DownloadsPanel::on_remove_clicked(
    GtkButton* button,
    gpointer user_data
) {
    auto* panel =
        static_cast<DownloadsPanel*>(
            user_data
        );

    if (!panel) {
        return;
    }

    // Solo remover de la UI.
    // DownloadManager mantiene el historial.
    GtkWidget* button_widget =
        GTK_WIDGET(button);

    GtkWidget* button_box =
        gtk_widget_get_parent(
            button_widget
        );

    if (!button_box) {
        return;
    }

    GtkWidget* row_box =
        gtk_widget_get_parent(
            button_box
        );

    if (!row_box) {
        return;
    }

    GtkWidget* list_row =
        gtk_widget_get_parent(
            row_box
        );

    if (list_row &&
        GTK_IS_LIST_BOX_ROW(list_row)) {

        gtk_list_box_remove(
            GTK_LIST_BOX(panel->list_box_),
            list_row
        );
    }
}

void DownloadsPanel::refresh() {
    if (!list_box_) {
        return;
    }

    populate_list();

    // list_box_ -> scrolled -> stack
    GtkWidget* scrolled =
        gtk_widget_get_parent(
            list_box_
        );

    if (!scrolled) {
        return;
    }

    GtkWidget* stack =
        gtk_widget_get_parent(
            scrolled
        );

    if (stack &&
        GTK_IS_STACK(stack)) {

        if (dm_->downloads().empty()) {

            gtk_stack_set_visible_child_name(
                GTK_STACK(stack),
                "empty"
            );

        } else {

            gtk_stack_set_visible_child_name(
                GTK_STACK(stack),
                "list"
            );
        }
    }
}