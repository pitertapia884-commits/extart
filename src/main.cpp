#include <gtk/gtk.h>
#include "browser.h"

static void on_activate(GtkApplication* app, gpointer user_data) {
    new Browser(app);
}

int main(int argc, char* argv[]) {
    GtkApplication* app = gtk_application_new("cl.extart.browser", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}