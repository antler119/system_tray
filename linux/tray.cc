#ifndef NATIVE_C
#define NATIVE_C

#include "tray.h"

#include <assert.h>
#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include "errors.h"
#include "menu.h"
#include "menu_manager.h"

constexpr char kInitSystemTray[] = "InitSystemTray";
constexpr char kSetSystemTrayInfo[] = "SetSystemTrayInfo";
constexpr char kSetContextMenu[] = "SetContextMenu";
constexpr char kPopupContextMenu[] = "PopupContextMenu";
constexpr char kGetTitle[] = "GetTitle";
constexpr char kDestroySystemTray[] = "DestroySystemTray";

namespace {

constexpr char kTrayIdKey[] = "tray_id";
constexpr char kTitleKey[] = "title";
constexpr char kIconPathKey[] = "iconpath";
constexpr char kToolTipKey[] = "tooltip";

}  // namespace

Tray::Tray(FlMethodChannel* channel,
           std::weak_ptr<MenuManager> menu_manager) noexcept
    : channel_(channel), menu_manager_(menu_manager) {}

Tray::~Tray() noexcept {
  destroy_indicator();

  channel_ = nullptr;
}

bool Tray::init_indicator_api() {
  bool ret = false;

  do {
    if (indicator_api_inited_) {
      ret = true;
      break;
    }

    void* handle = dlopen("libappindicator3.so.1", RTLD_LAZY);
    if (!handle) {
      break;
    }

    app_indicator_new_ = reinterpret_cast<app_indicator_new_fun>(
        dlsym(handle, "app_indicator_new"));
    app_indicator_set_status_ = reinterpret_cast<app_indicator_set_status_fun>(
        dlsym(handle, "app_indicator_set_status"));
    app_indicator_set_icon_full_ =
        reinterpret_cast<app_indicator_set_icon_full_func>(
            dlsym(handle, "app_indicator_set_icon_full"));
    app_indicator_set_attention_icon_full_ =
        reinterpret_cast<app_indicator_set_attention_icon_full_fun>(
            dlsym(handle, "app_indicator_set_attention_icon_full"));
    app_indicator_set_label_ = reinterpret_cast<app_indicator_set_label_func>(
        dlsym(handle, "app_indicator_set_label"));
    app_indicator_set_title_ = reinterpret_cast<app_indicator_set_title_func>(
        dlsym(handle, "app_indicator_set_title"));
    app_indicator_get_label_ = reinterpret_cast<app_indicator_get_label_func>(
        dlsym(handle, "app_indicator_get_label"));
    app_indicator_set_menu_ = reinterpret_cast<app_indicator_set_menu_fun>(
        dlsym(handle, "app_indicator_set_menu"));

    if (!app_indicator_new_ || !app_indicator_set_status_ ||
        !app_indicator_set_icon_full_ ||
        !app_indicator_set_attention_icon_full_ || !app_indicator_set_label_ ||
        !app_indicator_set_label_ || !app_indicator_get_label_ ||
        !app_indicator_set_menu_) {
      break;
    }

    indicator_api_inited_ = true;

    ret = true;
  } while (false);

  return ret;
}

bool Tray::create_indicator(const char* tray_id) {
  // printf("SystemTray::create_indicator tray_id: %s\n", tray_id);

  bool ret = false;

  do {
    if (!tray_id) {
      break;
    }

    if (!indicator_api_inited_) {
      break;
    }

    if (!app_indicator_) {
      app_indicator_ = app_indicator_new_(
          tray_id, "", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
      if (!app_indicator_) {
        break;
      }
    }

    app_indicator_set_status_(app_indicator_, APP_INDICATOR_STATUS_ACTIVE);
    ret = true;
  } while (false);

  return ret;
}

void Tray::destroy_indicator() {
  context_menu_id_ = -1;

  if (app_indicator_) {
    g_object_unref(G_OBJECT(app_indicator_));
    app_indicator_ = nullptr;
  }
}

void Tray::hide_indicator() {
  context_menu_id_ = -1;

  if (app_indicator_) {
    app_indicator_set_status_(app_indicator_, APP_INDICATOR_STATUS_PASSIVE);
  }
}

void Tray::handle_method_call(FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  // g_print("method call %s\n", method);

  if (strcmp(method, kInitSystemTray) == 0) {
    response = init_tray(args);
  } else if (strcmp(method, kSetSystemTrayInfo) == 0) {
    response = set_tray_info(args);
  } else if (strcmp(method, kSetContextMenu) == 0) {
    response = set_context_menu(args);
  } else if (strcmp(method, kPopupContextMenu) == 0) {
    response = popup_context_menu(args);
  } else if (strcmp(method, kGetTitle) == 0) {
    response = get_title(args);
  } else if (strcmp(method, kDestroySystemTray) == 0) {
    response = destroy_system_tray(args);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error)) {
    g_warning("Failed to send method call response: %s", error->message);
  }
}

FlMethodResponse* Tray::init_tray(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    const gchar* tray_id = nullptr;

    FlValue* tray_id_value = fl_value_lookup_string(args, kTrayIdKey);
    if (tray_id_value &&
        fl_value_get_type(tray_id_value) == FL_VALUE_TYPE_STRING) {
      tray_id = fl_value_get_string(tray_id_value);
    }

    if (!init_tray(tray_id)) {
      break;
    }

    response = set_tray_info(args);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* Tray::set_tray_info(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
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

    result = fl_value_new_bool(set_tray_info(title, icon_path, tool_tip));

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* Tray::set_context_menu(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_INT) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    set_context_menu(fl_value_get_int(args));

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

FlMethodResponse* Tray::popup_context_menu(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(TRUE);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

FlMethodResponse* Tray::get_title(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_string("");
  FlMethodResponse* response = nullptr;

  do {
    if (!app_indicator_) {
      break;
    }

    const gchar* title = app_indicator_get_label_(app_indicator_);
    result = fl_value_new_string(title ? title : "");

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

FlMethodResponse* Tray::destroy_system_tray(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_string(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (!app_indicator_) {
      break;
    }

    hide_indicator();

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

bool Tray::init_tray(const char* tray_id) {
  bool ret = false;

  do {
    if (!init_indicator_api()) {
      break;
    }

    if (!create_indicator(tray_id)) {
      break;
    }

    ret = true;
  } while (false);

  return ret;
}

bool Tray::set_tray_info(const char* title,
                         const char* icon_path,
                         const char* toolTip) {
  printf(
      "SystemTray::set_system_tray_info title: %s, icon_path: %s, toolTip: "
      "%s\n",
      title, icon_path, toolTip);

  bool ret = false;

  do {
    if (!app_indicator_) {
      break;
    }

    if (icon_path) {
      if (strlen(icon_path)) {
        app_indicator_set_status_(app_indicator_, APP_INDICATOR_STATUS_ACTIVE);
        app_indicator_set_icon_full_(app_indicator_, icon_path, "icon");
      } else {
        app_indicator_set_status_(app_indicator_, APP_INDICATOR_STATUS_PASSIVE);
      }
    }

    if (title) {
      app_indicator_set_label_(app_indicator_, title, nullptr);
    }

    ret = true;
  } while (false);

  return ret;
}

void Tray::set_context_menu(int64_t context_menu_id) {
  context_menu_id_ = context_menu_id;

  do {
    if (menu_manager_.expired()) {
      break;
    }

    std::shared_ptr<MenuManager> menu_manager = menu_manager_.lock();
    std::shared_ptr<Menu> menu = menu_manager->get_menu(get_context_menu_id());
    if (!menu) {
      break;
    }

    if (!app_indicator_) {
      break;
    }

    GtkWidget* system_menu = menu->get_menu();

    gtk_widget_show_all(system_menu);
    app_indicator_set_menu_(app_indicator_, GTK_MENU(system_menu));

  } while (false);
}

int64_t Tray::get_context_menu_id() const {
  return context_menu_id_;
}

#endif  // NATIVE_C