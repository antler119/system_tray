#include "include/system_tray/system_tray_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include <cstring>
#include <memory>

#include "app_window.h"

namespace {

constexpr char kChannelNameAppWindow[] = "flutter/system_tray/app_window";

}  // namespace

#define SYSTEM_TRAY_PLUGIN(obj)                                     \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), system_tray_plugin_get_type(), \
                              SystemTrayPlugin))

struct _SystemTrayPlugin {
  GObject parent_instance;

  FlPluginRegistrar* registrar;

  FlMethodChannel* channel_app_window = nullptr;

  std::unique_ptr<AppWindow> app_window;
};

G_DEFINE_TYPE(SystemTrayPlugin, system_tray_plugin, g_object_get_type())

// Called when a method call is received from Flutter.
static void system_tray_plugin_handle_method_call(SystemTrayPlugin* self,
                                                  FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);

  if (strcmp(method, kInitAppWindow) == 0 ||
      strcmp(method, kShowAppWindow) == 0 ||
      strcmp(method, kHideAppWindow) == 0 ||
      strcmp(method, kCloseAppWindow) == 0) {
    self->app_window->handle_method_call(method_call);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
    g_autoptr(GError) error = nullptr;
    if (!fl_method_call_respond(method_call, response, &error)) {
      g_warning("Failed to send method call response: %s", error->message);
    }
  }
}

static void system_tray_plugin_dispose(GObject* object) {
  SystemTrayPlugin* self = SYSTEM_TRAY_PLUGIN(object);

  g_clear_object(&self->registrar);
  g_clear_object(&self->channel_app_window);

  G_OBJECT_CLASS(system_tray_plugin_parent_class)->dispose(object);
}

static void system_tray_plugin_class_init(SystemTrayPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = system_tray_plugin_dispose;
}

static void system_tray_plugin_init(SystemTrayPlugin* self) {}

static void method_call_cb(FlMethodChannel* channel,
                           FlMethodCall* method_call,
                           gpointer user_data) {
  SystemTrayPlugin* plugin = SYSTEM_TRAY_PLUGIN(user_data);
  system_tray_plugin_handle_method_call(plugin, method_call);
}

void system_tray_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  SystemTrayPlugin* plugin = SYSTEM_TRAY_PLUGIN(
      g_object_new(system_tray_plugin_get_type(), nullptr));

  plugin->registrar = FL_PLUGIN_REGISTRAR(g_object_ref(registrar));

  g_autoptr(FlStandardMethodCodec) codec_app_window =
      fl_standard_method_codec_new();
  plugin->channel_app_window = fl_method_channel_new(
      fl_plugin_registrar_get_messenger(registrar), kChannelNameAppWindow,
      FL_METHOD_CODEC(codec_app_window));

  plugin->app_window = std::make_unique<AppWindow>(registrar, plugin->channel_app_window);

  fl_method_channel_set_method_call_handler(
      plugin->channel_app_window, method_call_cb, g_object_ref(plugin),
      g_object_unref);

  g_object_unref(plugin);
}
