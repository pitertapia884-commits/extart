#include "history_panel.hpp"

#include "history.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

HistoryPanel::HistoryPanel(History* history)
    : history_(history) {
}

HistoryPanel::~HistoryPanel() = default;

GtkWidget* HistoryPanel::create_panel() {
    GtkWidget* box =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_widget_set_margin_top(box, 12);
    gtk_widget_set_margin_bottom(box, 12);

    // Header: búsqueda + limpiar
    GtkWidget* header =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    search_entry_ = gtk_search_entry_new();

    gtk_widget_set_hexpand(
        search_entry_,
        TRUE
    );

    gtk_widget_set_tooltip_text(
        search_entry_,
        "Search history"
    );

    GtkWidget* clear_button =
        gtk_button_new_with_label(
            "Clear history"
        );

    gtk_widget_add_css_class(
        clear_button,
        "destructive-action"
    );

    g_signal_connect(
        clear_button,
        "clicked",
        G_CALLBACK(on_clear_clicked),
        this
    );

    gtk_box_append(
        GTK_BOX(header),
        search_entry_
    );

    gtk_box_append(
        GTK_BOX(header),
        clear_button
    );

    // Lista con scroll
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

    g_signal_connect(
        list_box_,
        "row-activated",
        G_CALLBACK(on_row_activated),
        this
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scrolled),
        list_box_
    );

    // Estado vacío
    empty_label_ =
        gtk_label_new(
            "No history yet"
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

    gtk_widget_set_vexpand(
        stack,
        TRUE
    );

    gtk_widget_set_hexpand(
        stack,
        TRUE
    );

    gtk_stack_set_visible_child_name(
        GTK_STACK(stack),
        "empty"
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

    // Actualizar visibilidad inicial
    if (!history_->entries().empty()) {
        gtk_stack_set_visible_child_name(
            GTK_STACK(stack),
            "list"
        );
    }

    return box;
}

void HistoryPanel::set_on_entry_activated(
    OnEntryActivated callback
) {
    on_entry_activated_ =
        std::move(callback);
}

void HistoryPanel::populate_list() {
    if (!list_box_) {
        return;
    }

    // Limpiar lista actual
    while (GtkWidget* child =
               gtk_widget_get_first_child(
                   list_box_
               )) {

        gtk_list_box_remove(
            GTK_LIST_BOX(list_box_),
            child
        );
    }

    const auto& entries =
        history_->entries();

    if (entries.empty()) {
        return;
    }

    // Copia para ordenar sin modificar el historial original
    auto sorted_entries = entries;

    std::sort(
        sorted_entries.begin(),
        sorted_entries.end(),
        [](const HistoryEntry& a,
           const HistoryEntry& b) {
            return a.timestamp >
                   b.timestamp;
        }
    );

    for (const auto& entry :
         sorted_entries) {

        GtkWidget* row =
            gtk_box_new(
                GTK_ORIENTATION_VERTICAL,
                4
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

        // Título
        GtkWidget* title_label =
            gtk_label_new(
                entry.title.c_str()
            );

        gtk_label_set_wrap(
            GTK_LABEL(title_label),
            TRUE
        );

        gtk_label_set_xalign(
            GTK_LABEL(title_label),
            0.0f
        );

        gtk_widget_add_css_class(
            title_label,
            "title-4"
        );

        // URL
        GtkWidget* url_label =
            gtk_label_new(
                entry.url.c_str()
            );

        gtk_label_set_wrap(
            GTK_LABEL(url_label),
            TRUE
        );

        gtk_label_set_xalign(
            GTK_LABEL(url_label),
            0.0f
        );

        gtk_label_set_ellipsize(
            GTK_LABEL(url_label),
            PANGO_ELLIPSIZE_MIDDLE
        );

        gtk_widget_add_css_class(
            url_label,
            "dim-label"
        );

        // Fecha/hora
        std::time_t timestamp =
            entry.timestamp;

        std::tm* local_time =
            std::localtime(
                &timestamp
            );

        std::ostringstream time_stream;

        if (local_time != nullptr) {
            time_stream
                << std::put_time(
                       local_time,
                       "%Y-%m-%d %H:%M"
                   );
        }

        GtkWidget* time_label =
            gtk_label_new(
                time_stream.str().c_str()
            );

        gtk_label_set_xalign(
            GTK_LABEL(time_label),
            0.0f
        );

        gtk_widget_add_css_class(
            time_label,
            "dim-label"
        );

        gtk_widget_add_css_class(
            time_label,
            "small-text"
        );

        gtk_box_append(
            GTK_BOX(row),
            title_label
        );

        gtk_box_append(
            GTK_BOX(row),
            url_label
        );

        gtk_box_append(
            GTK_BOX(row),
            time_label
        );

        // Fila real del GtkListBox
        GtkListBoxRow* list_row =
            GTK_LIST_BOX_ROW(
                gtk_list_box_row_new()
            );

        gtk_list_box_row_set_child(
            list_row,
            row
        );

        // Guardar URL para cuando se haga click
        g_object_set_data_full(
            G_OBJECT(list_row),
            "url",
            g_strdup(
                entry.url.c_str()
            ),
            g_free
        );

        gtk_list_box_append(
            GTK_LIST_BOX(list_box_),
            GTK_WIDGET(list_row)
        );
    }
}

void HistoryPanel::on_row_activated(
    GtkListBox*,
    GtkListBoxRow* row,
    gpointer user_data
) {
    auto* panel =
        static_cast<HistoryPanel*>(
            user_data
        );

    if (!panel ||
        !panel->on_entry_activated_) {
        return;
    }

    const char* url =
        static_cast<const char*>(
            g_object_get_data(
                G_OBJECT(row),
                "url"
            )
        );

    if (url != nullptr) {
        panel->on_entry_activated_(
            std::string(url)
        );
    }
}

void HistoryPanel::on_clear_clicked(
    GtkButton*,
    gpointer user_data
) {
    auto* panel =
        static_cast<HistoryPanel*>(
            user_data
        );

    if (!panel ||
        !panel->history_) {
        return;
    }

    panel->history_->clear();

    panel->refresh();
}

void HistoryPanel::refresh() {
    if (!list_box_) {
        return;
    }

    populate_list();

    GtkWidget* parent =
        gtk_widget_get_parent(
            list_box_
        );

    // list_box_ -> scrolled -> stack
    if (parent) {
        GtkWidget* stack =
            gtk_widget_get_parent(
                parent
            );

        if (stack &&
            GTK_IS_STACK(stack)) {

            if (history_->entries().empty()) {
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
}

void HistoryPanel::clear_all() {
    if (!history_) {
        return;
    }

    history_->clear();

    refresh();
}