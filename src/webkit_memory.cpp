#include "webkit_memory.hpp"

#include <webkit/webkit.h>
#include <glib.h>

void optimize_webkit_memory() {
    WebKitWebContext* context = webkit_web_context_get_default();

    if (!context) {
        return;
    }

    // Usar el modelo de caché para reducir el uso de recursos
    // en comparación con un navegador de propósito general.
    webkit_web_context_set_cache_model(
        context,
        WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER
    );

    // Crear las opciones de WebKit.
    WebKitSettings* settings = webkit_settings_new();

    // Evitar que JavaScript abra ventanas automáticamente.
    webkit_settings_set_javascript_can_open_windows_automatically(
        settings,
        FALSE
    );

    // Mantener la carga automática de imágenes.
    g_object_set(
        settings,
        "auto-load-images",
        TRUE,
        nullptr
    );

    // Desactivar WebGL para reducir consumo de recursos.
    webkit_settings_set_enable_webgl(
        settings,
        FALSE
    );

    g_object_unref(settings);
}