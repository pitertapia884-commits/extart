#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <string>

static GtkWidget* webview;
static GtkWidget* url_bar;

static void navigate(GtkWidget* widget, gpointer user_data) {
    const char* url = gtk_editable_get_text(GTK_EDITABLE(url_bar));
    std::string uri(url);

    if (uri.find("http://") != 0 && uri.find("https://") != 0) {
        uri = "https://" + uri;
    }

    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), uri.c_str());
}

static void on_back(GtkWidget* widget, gpointer user_data) {
    webkit_web_view_go_back(WEBKIT_WEB_VIEW(webview));
}

static void on_forward(GtkWidget* widget, gpointer user_data) {
    webkit_web_view_go_forward(WEBKIT_WEB_VIEW(webview));
}

static void on_reload(GtkWidget* widget, gpointer user_data) {
    webkit_web_view_reload(WEBKIT_WEB_VIEW(webview));
}

static void on_load_changed(WebKitWebView* wv, WebKitLoadEvent event, gpointer user_data) {
    if (event == WEBKIT_LOAD_COMMITTED) {
        const char* uri = webkit_web_view_get_uri(wv);
        if (uri) {
            gtk_editable_set_text(GTK_EDITABLE(url_bar), uri);
        }
    }
}

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "EXTART");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget* navbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_margin_start(navbar, 6);
    gtk_widget_set_margin_end(navbar, 6);
    gtk_widget_set_margin_top(navbar, 6);
    gtk_widget_set_margin_bottom(navbar, 6);

    GtkWidget* btn_back    = gtk_button_new_with_label("←");
    GtkWidget* btn_forward = gtk_button_new_with_label("→");
    GtkWidget* btn_reload  = gtk_button_new_with_label("↺");

    url_bar = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(url_bar), "https://duckduckgo.com");
    gtk_widget_set_hexpand(url_bar, TRUE);

    g_signal_connect(url_bar,     "activate", G_CALLBACK(navigate),   NULL);
    g_signal_connect(btn_back,    "clicked",  G_CALLBACK(on_back),    NULL);
    g_signal_connect(btn_forward, "clicked",  G_CALLBACK(on_forward), NULL);
    g_signal_connect(btn_reload,  "clicked",  G_CALLBACK(on_reload),  NULL);

    gtk_box_append(GTK_BOX(navbar), btn_back);
    gtk_box_append(GTK_BOX(navbar), btn_forward);
    gtk_box_append(GTK_BOX(navbar), btn_reload);
    gtk_box_append(GTK_BOX(navbar), url_bar);

    webview = webkit_web_view_new();
    gtk_widget_set_vexpand(webview, TRUE);
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), "https://duckduckgo.com");

    g_signal_connect(webview, "load-changed", G_CALLBACK(on_load_changed), NULL);

    gtk_box_append(GTK_BOX(vbox), navbar);
    gtk_box_append(GTK_BOX(vbox), webview);

    gtk_window_set_child(GTK_WINDOW(window), vbox);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char* argv[]) {
    GtkApplication* app = gtk_application_new("cl.extart.browser", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}