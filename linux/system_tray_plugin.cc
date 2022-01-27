#include "include/system_tray/system_tray_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include <cstring>
#include <memory>

#include "app_window.h"
#include "tray.h"

const static char kBadArgumentsError[] = "Bad Arguments";
const static char kOutOfMemoryError[] = "Out of memory";

const static char kChannelName[] = "flutter/system_tray";

const static char kInitSystemTray[] = "InitSystemTray";
const static char kSetSystemTrayInfo[] = "SetSystemTrayInfo";
const static char kSetContextMenu[] = "SetContextMenu";
const static char kMenuItemSelectedCallbackMethod[] =
    "MenuItemSelectedCallback";

const static char kTitleKey[] = "title";
const static char kIconPathKey[] = "iconpath";
const static char kToolTipKey[] = "tooltip";
const static char kIdKey[] = "id";
const static char kTypeKey[] = "type";
const static char kLabelKey[] = "label";
const static char kSeparatorKey[] = "separator";
const static char kSubMenuKey[] = "submenu";
const static char kEnabledKey[] = "enabled";

const static char kChannelAppWindowName[] = "flutter/system_tray/app_window";

const static char kInitAppWindow[] = "InitAppWindow";
const static char kShowAppWindow[] = "ShowAppWindow";
const static char kHideAppWindow[] = "HideAppWindow";
const static char kCloseAppWindow[] = "CloseAppWindow";

#define SYSTEM_TRAY_PLUGIN(obj)                                     \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), system_tray_plugin_get_type(), \
                              SystemTrayPlugin))

struct _SystemTrayPlugin {
  GObject parent_instance;

  FlPluginRegistrar* registrar;

  FlMethodChannel* channel;

  FlMethodChannel* channel_app_window;

  std::unique_ptr<SystemTray> system_tray;

  std::unique_ptr<AppWindow> app_window;
};

G_DEFINE_TYPE(SystemTrayPlugin, system_tray_plugin, g_object_get_type())

SystemTrayPlugin* g_plugin = nullptr;

static void tray_callback(GtkMenuItem* item, gpointer user_data) {
  int64_t id = GPOINTER_TO_INT(user_data);

  g_print("tray_callback id:%ld\n", id);

  g_autoptr(FlValue) result = fl_value_new_int(id);
  fl_method_channel_invoke_method(g_plugin->channel,
                                  kMenuItemSelectedCallbackMethod, result,
                                  nullptr, nullptr, nullptr);
}

static FlMethodResponse* init_system_tray(SystemTrayPlugin* self,
                                          FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new(kBadArgumentsError, "not map", nullptr));
      break;
    }

    const gchar* title = nullptr;
    const gchar* icon_path = nullptr;
    const gchar* tool_tip = nullptr;

    FlValue* title_value = fl_value_lookup_string(args, kTitleKey);
    if (title_value && fl_value_get_type(title_value) == FL_VALUE_TYPE_STRING) {
      title = fl_value_get_string(title_value);
    }

    FlValue* icon_path_value = fl_value_lookup_string(args, kIconPathKey);
    if (icon_path_value &&
        fl_value_get_type(icon_path_value) == FL_VALUE_TYPE_STRING) {
      icon_path = fl_value_get_string(icon_path_value);
    }

    FlValue* tooltip_value = fl_value_lookup_string(args, kToolTipKey);
    if (tooltip_value &&
        fl_value_get_type(tooltip_value) == FL_VALUE_TYPE_STRING) {
      tool_tip = fl_value_get_string(tooltip_value);
    }

    result = fl_value_new_bool(
        self->system_tray->init_system_tray(title, icon_path, tool_tip));

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

static FlMethodResponse* set_system_tray_info(SystemTrayPlugin* self,
                                              FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new(kBadArgumentsError, "not map", nullptr));
      break;
    }

    const gchar* title = nullptr;
    const gchar* icon_path = nullptr;
    const gchar* tool_tip = nullptr;

    FlValue* title_value = fl_value_lookup_string(args, kTitleKey);
    if (title_value && fl_value_get_type(title_value) == FL_VALUE_TYPE_STRING) {
      title = fl_value_get_string(title_value);
    }

    FlValue* icon_path_value = fl_value_lookup_string(args, kIconPathKey);
    if (icon_path_value &&
        fl_value_get_type(icon_path_value) == FL_VALUE_TYPE_STRING) {
      icon_path = fl_value_get_string(icon_path_value);
    }

    FlValue* tooltip_value = fl_value_lookup_string(args, kToolTipKey);
    if (tooltip_value &&
        fl_value_get_type(tooltip_value) == FL_VALUE_TYPE_STRING) {
      tool_tip = fl_value_get_string(tooltip_value);
    }

    result = fl_value_new_bool(
        self->system_tray->set_system_tray_info(title, icon_path, tool_tip));

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

static GtkWidget* value_to_menu(SystemTrayPlugin* self, FlValue* value);

static GtkWidget* value_to_menu_item(SystemTrayPlugin* self, FlValue* value) {
  if (fl_value_get_type(value) != FL_VALUE_TYPE_MAP) {
    return nullptr;
  }

  FlValue* type_value = fl_value_lookup_string(value, kTypeKey);
  if (type_value == nullptr ||
      fl_value_get_type(type_value) != FL_VALUE_TYPE_STRING) {
    return nullptr;
  }

  GtkWidget* menuItem = nullptr;

  const gchar* type = fl_value_get_string(type_value);

  // g_print("value_to_menu_item type:%s\n", type);

  if (strcmp(type, kSeparatorKey) == 0) {
    menuItem = gtk_separator_menu_item_new();
  } else if (strcmp(type, kSubMenuKey) == 0) {
    FlValue* label_value = fl_value_lookup_string(value, kLabelKey);
    if (label_value != nullptr &&
        fl_value_get_type(label_value) == FL_VALUE_TYPE_STRING) {
      // g_print("value_to_menu_item submenu label:%s\n",
      //         fl_value_get_string(label_value));
      menuItem = gtk_menu_item_new_with_label(fl_value_get_string(label_value));
      GtkWidget* subMenu =
          value_to_menu(self, fl_value_lookup_string(value, kSubMenuKey));
      if (subMenu == nullptr) {
        return nullptr;
      }
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);
    }
  } else {
    FlValue* label_value = fl_value_lookup_string(value, kLabelKey);
    if (label_value != nullptr &&
        fl_value_get_type(label_value) == FL_VALUE_TYPE_STRING) {
      // g_print("value_to_menu_item label:%s\n",
      //         fl_value_get_string(label_value));
      menuItem = gtk_menu_item_new_with_label(fl_value_get_string(label_value));

      FlValue* enabled_value = fl_value_lookup_string(value, kEnabledKey);
      if (enabled_value != nullptr &&
          fl_value_get_type(enabled_value) == FL_VALUE_TYPE_BOOL) {
        gtk_widget_set_sensitive(
            menuItem, fl_value_get_bool(enabled_value) ? TRUE : FALSE);
      }

      FlValue* id_value = fl_value_lookup_string(value, kIdKey);
      if (id_value != nullptr &&
          fl_value_get_type(id_value) == FL_VALUE_TYPE_INT) {
        g_signal_connect(G_OBJECT(menuItem), "activate",
                         G_CALLBACK(tray_callback),
                         GINT_TO_POINTER(fl_value_get_int(id_value)));
      }
    }
  }

  return menuItem;
}

static GtkWidget* value_to_menu(SystemTrayPlugin* self, FlValue* value) {
  if (fl_value_get_type(value) != FL_VALUE_TYPE_LIST) {
    return nullptr;
  }

  GtkWidget* menu = gtk_menu_new();

  for (size_t i = 0; i < fl_value_get_length(value); ++i) {
    GtkWidget* menuItem =
        value_to_menu_item(self, fl_value_get_list_value(value, i));
    if (menuItem == nullptr) {
      return nullptr;
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(menuItem));
  }
  return GTK_WIDGET(menu);
}

static FlMethodResponse* set_context_menu(SystemTrayPlugin* self,
                                          FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    GtkWidget* menu = value_to_menu(self, args);
    if (menu == nullptr) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          kBadArgumentsError, "Menu list missing or malformed", nullptr));
      break;
    }

    self->system_tray->set_context_menu(GTK_WIDGET(menu));

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

static FlMethodResponse* init_app_window(SystemTrayPlugin* self,
                                         FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (self->app_window) {
      result = fl_value_new_bool(TRUE);
      break;
    }

    FlView* view = fl_plugin_registrar_get_view(self->registrar);
    if (view == nullptr) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          kBadArgumentsError, "Expected view", fl_value_new_bool(FALSE)));
      break;
    }

    GtkWindow* window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(view)));
    if (window == nullptr) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          kBadArgumentsError, "Expected window", fl_value_new_bool(FALSE)));
      break;
    }

    self->app_window = std::make_unique<AppWindow>();
    if (!self->app_window) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          kOutOfMemoryError, "Out of memory", fl_value_new_bool(FALSE)));
      break;
    }

    if (!self->app_window->init_app_window(window)) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          kBadArgumentsError, "Unable to init appwindow",
          fl_value_new_bool(FALSE)));
      break;
    }

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

static FlMethodResponse* show_app_window(SystemTrayPlugin* self,
                                         FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (!self->app_window) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          kBadArgumentsError, "Expected app window", fl_value_new_bool(FALSE)));
      break;
    }

    self->app_window->show_app_window();

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

static FlMethodResponse* hide_app_window(SystemTrayPlugin* self,
                                         FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (!self->app_window) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          kBadArgumentsError, "Expected app window", fl_value_new_bool(FALSE)));
      break;
    }

    self->app_window->hide_app_window();

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

static FlMethodResponse* close_app_window(SystemTrayPlugin* self,
                                          FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (!self->app_window) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          kBadArgumentsError, "Expected app window", fl_value_new_bool(FALSE)));
      break;
    }

    self->app_window->close_app_window();

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

// Called when a method call is received from Flutter.
static void system_tray_plugin_handle_method_call(SystemTrayPlugin* self,
                                                  FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  // g_print("method call %s\n", method);

  if (strcmp(method, kInitSystemTray) == 0) {
    response = init_system_tray(self, args);
  } else if (strcmp(method, kSetSystemTrayInfo) == 0) {
    response = set_system_tray_info(self, args);
  } else if (strcmp(method, kSetContextMenu) == 0) {
    response = set_context_menu(self, args);
  } else if (strcmp(method, kInitAppWindow) == 0) {
    response = init_app_window(self, args);
  } else if (strcmp(method, kShowAppWindow) == 0) {
    response = show_app_window(self, args);
  } else if (strcmp(method, kHideAppWindow) == 0) {
    response = hide_app_window(self, args);
  } else if (strcmp(method, kCloseAppWindow) == 0) {
    response = close_app_window(self, args);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error))
    g_warning("Failed to send method call response: %s", error->message);
}

static void system_tray_plugin_dispose(GObject* object) {
  SystemTrayPlugin* self = SYSTEM_TRAY_PLUGIN(object);
  g_print("system_tray_plugin_dispose self: %p\n", self);

  g_clear_object(&self->registrar);
  g_clear_object(&self->channel);

  G_OBJECT_CLASS(system_tray_plugin_parent_class)->dispose(object);
}

static void system_tray_plugin_class_init(SystemTrayPluginClass* klass) {
  g_print("system_tray_plugin_class_init klass: %p\n", klass);
  G_OBJECT_CLASS(klass)->dispose = system_tray_plugin_dispose;
}

static void system_tray_plugin_init(SystemTrayPlugin* self) {
  g_print("system_tray_plugin_init self: %p\n", self);
  g_plugin = self;
}

static void method_call_cb(FlMethodChannel* channel,
                           FlMethodCall* method_call,
                           gpointer user_data) {
  SystemTrayPlugin* plugin = SYSTEM_TRAY_PLUGIN(user_data);
  system_tray_plugin_handle_method_call(plugin, method_call);
}

void system_tray_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  SystemTrayPlugin* plugin =
      SYSTEM_TRAY_PLUGIN(g_object_new(system_tray_plugin_get_type(), nullptr));

  plugin->registrar = FL_PLUGIN_REGISTRAR(g_object_ref(registrar));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  plugin->channel =
      fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                            kChannelName, FL_METHOD_CODEC(codec));

  g_autoptr(FlStandardMethodCodec) app_window_codec =
      fl_standard_method_codec_new();
  plugin->channel_app_window = fl_method_channel_new(
      fl_plugin_registrar_get_messenger(registrar), kChannelAppWindowName,
      FL_METHOD_CODEC(app_window_codec));

  plugin->system_tray = std::make_unique<SystemTray>();

  fl_method_channel_set_method_call_handler(
      plugin->channel, method_call_cb, g_object_ref(plugin), g_object_unref);

  fl_method_channel_set_method_call_handler(
      plugin->channel_app_window, method_call_cb, g_object_ref(plugin),
      g_object_unref);

  g_object_unref(plugin);
}