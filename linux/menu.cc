#include "menu.h"

#include <memory>

#include "errors.h"

namespace {

constexpr char kMenuIdKey[] = "menu_id";
constexpr char kMenuItemIdKey[] = "menu_item_id";
constexpr char kMenuListKey[] = "menu_list";
constexpr char kIdKey[] = "id";
constexpr char kTypeKey[] = "type";
constexpr char kSeparatorKey[] = "separator";
constexpr char kSubMenuKey[] = "submenu";
constexpr char kCheckboxKey[] = "checkbox";
constexpr char kLabelKey[] = "label";
constexpr char kImageKey[] = "image";
constexpr char kEnabledKey[] = "enabled";
constexpr char kCheckedKey[] = "checked";

constexpr char kMenuItemSelectedCallbackMethod[] = "MenuItemSelectedCallback";

struct TrayCallbackData {
  Menu* menu;
  int64_t menu_id;
  int64_t menu_item_id;
};

}  // namespace

Menu::Menu(FlMethodChannel* channel, int menu_id) noexcept
    : channel_(channel), menu_id_(menu_id) {}

Menu::~Menu() noexcept {
  // printf("~Menu this: %p\n", this);
}

bool Menu::create_context_menu(FlValue* args) {
  bool result = false;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      break;
    }

    FlValue* list_value = fl_value_lookup_string(args, kMenuListKey);
    if (!list_value || fl_value_get_type(list_value) != FL_VALUE_TYPE_LIST) {
      break;
    }

    GtkWidget* gtk_menu = value_to_menu(menu_id(), list_value);
    if (!gtk_menu) {
      break;
    }

    gtk_menu_ = GTK_WIDGET(g_object_ref(gtk_menu));

    result = true;

  } while (false);

  return result;
}

FlMethodResponse* Menu::set_label(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    FlValue* menu_item_id_value = fl_value_lookup_string(args, kMenuItemIdKey);
    if (!menu_item_id_value ||
        fl_value_get_type(menu_item_id_value) != FL_VALUE_TYPE_INT) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    int64_t menu_item_id = fl_value_get_int(menu_item_id_value);

    const gchar* label = nullptr;
    FlValue* label_value = fl_value_lookup_string(args, kLabelKey);
    if (label_value && fl_value_get_type(label_value) == FL_VALUE_TYPE_STRING) {
      label = fl_value_get_string(label_value);
    }

    set_label(menu_item_id, label);

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* Menu::set_image(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    FlValue* menu_item_id_value = fl_value_lookup_string(args, kMenuItemIdKey);
    if (!menu_item_id_value ||
        fl_value_get_type(menu_item_id_value) != FL_VALUE_TYPE_INT) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    int64_t menu_item_id = fl_value_get_int(menu_item_id_value);

    const gchar* image = nullptr;
    FlValue* image_value = fl_value_lookup_string(args, kImageKey);
    if (image_value && fl_value_get_type(image_value) == FL_VALUE_TYPE_STRING) {
      image = fl_value_get_string(image_value);
    }

    set_image(menu_item_id, image);

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* Menu::set_enable(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    FlValue* menu_item_id_value = fl_value_lookup_string(args, kMenuItemIdKey);
    if (!menu_item_id_value ||
        fl_value_get_type(menu_item_id_value) != FL_VALUE_TYPE_INT) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    int64_t menu_item_id = fl_value_get_int(menu_item_id_value);

    bool enable = true;
    FlValue* enable_value = fl_value_lookup_string(args, kEnabledKey);
    if (enable_value &&
        fl_value_get_type(enable_value) == FL_VALUE_TYPE_STRING) {
      enable = fl_value_get_bool(enable_value);
    }

    set_enable(menu_item_id, enable);

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* Menu::set_check(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "not map", nullptr));
      break;
    }

    FlValue* menu_item_id_value = fl_value_lookup_string(args, kMenuItemIdKey);
    if (!menu_item_id_value ||
        fl_value_get_type(menu_item_id_value) != FL_VALUE_TYPE_INT) {
      break;
    }

    int64_t menu_item_id = fl_value_get_int(menu_item_id_value);

    bool checked = true;
    FlValue* checked_value = fl_value_lookup_string(args, kCheckedKey);
    if (checked_value &&
        fl_value_get_type(checked_value) == FL_VALUE_TYPE_STRING) {
      checked = fl_value_get_bool(checked_value);
    }

    set_check(menu_item_id, checked);

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

void Menu::set_label(int64_t menu_item_id, const char* label) {}
void Menu::set_image(int64_t menu_item_id, const char* image) {}
void Menu::set_enable(int64_t menu_item_id, bool enabled) {}
void Menu::set_check(int64_t menu_item_id, bool checked) {}

int64_t Menu::menu_id() const {
  return menu_id_;
}

GtkWidget* Menu::get_menu() const {
  return gtk_menu_;
}

// static
void Menu::menu_item_callback(GtkMenuItem* item, gpointer user_data) {
  TrayCallbackData* callback_data =
      reinterpret_cast<TrayCallbackData*>(user_data);
  if (callback_data && callback_data->menu) {
    callback_data->menu->handle_menu_item_callback(item, callback_data);
  }
}

void Menu::handle_menu_item_callback(GtkMenuItem* item, gpointer user_data) {
  TrayCallbackData* callback_data =
      reinterpret_cast<TrayCallbackData*>(user_data);

  // g_print("handle_menu_item_callback menu_id:%ld, menu_item_id:%ld\n",
  //         callback_data->menu_id, callback_data->menu_item_id);

  g_autoptr(FlValue) result = fl_value_new_map();
  fl_value_set_string_take(result, kMenuIdKey,
                           fl_value_new_int(callback_data->menu_id));
  fl_value_set_string_take(result, kMenuItemIdKey,
                           fl_value_new_int(callback_data->menu_item_id));
  fl_method_channel_invoke_method(channel_, kMenuItemSelectedCallbackMethod,
                                  result, nullptr, nullptr, nullptr);
}

GtkWidget* Menu::value_to_menu(int64_t menu_id, FlValue* value) {
  if (fl_value_get_type(value) != FL_VALUE_TYPE_LIST) {
    return nullptr;
  }

  GtkWidget* menu = gtk_menu_new();

  for (size_t i = 0; i < fl_value_get_length(value); ++i) {
    GtkWidget* menu_item =
        value_to_menu_item(menu_id, fl_value_get_list_value(value, i));
    if (menu_item == nullptr) {
      return nullptr;
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), GTK_WIDGET(menu_item));
  }
  return GTK_WIDGET(menu);
}

GtkWidget* Menu::value_to_menu_item(int64_t menu_id, FlValue* value) {
  if (fl_value_get_type(value) != FL_VALUE_TYPE_MAP) {
    return nullptr;
  }

  FlValue* type_value = fl_value_lookup_string(value, kTypeKey);
  if (type_value == nullptr ||
      fl_value_get_type(type_value) != FL_VALUE_TYPE_STRING) {
    return nullptr;
  }

  GtkWidget* menu_item = nullptr;

  const gchar* type = fl_value_get_string(type_value);

  if (strcmp(type, kSeparatorKey) == 0) {
    menu_item = gtk_separator_menu_item_new();
  } else {
    const gchar* label = nullptr;
    FlValue* label_value = fl_value_lookup_string(value, kLabelKey);
    if (label_value != nullptr &&
        fl_value_get_type(label_value) == FL_VALUE_TYPE_STRING) {
      label = fl_value_get_string(label_value);
    }

    FlValue* image_value = fl_value_lookup_string(value, kImageKey);
    if (image_value != nullptr &&
        fl_value_get_type(image_value) == FL_VALUE_TYPE_STRING) {
      const gchar* image = fl_value_get_string(image_value);

      // g_print("value_to_menu_item type:%s, label:%s, image:%s\n", type,
      // label, image);

      menu_item = gtk_menu_item_new();

      GtkWidget* box_widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
      GtkWidget* icon_widget = gtk_image_new_from_file(image);
      GtkWidget* label_widget = gtk_label_new(label);

      gtk_container_add(GTK_CONTAINER(box_widget), icon_widget);
      gtk_container_add(GTK_CONTAINER(box_widget), label_widget);
      gtk_container_add(GTK_CONTAINER(menu_item), box_widget);

      gtk_widget_show_all(menu_item);
    } else {
      g_print("value_to_menu_item type:%s, label:%s\n", type, label);
    }

    if (!menu_item) {
      if (strcmp(type, kCheckboxKey) == 0) {
        menu_item = gtk_check_menu_item_new_with_label(label);
      } else {
        menu_item = gtk_menu_item_new_with_label(label);
      }
    }

    FlValue* enabled_value = fl_value_lookup_string(value, kEnabledKey);
    if (enabled_value != nullptr &&
        fl_value_get_type(enabled_value) == FL_VALUE_TYPE_BOOL) {
      gtk_widget_set_sensitive(menu_item,
                               fl_value_get_bool(enabled_value) ? TRUE : FALSE);
    }

    if (strcmp(type, kSubMenuKey) == 0) {
      GtkWidget* subMenu =
          value_to_menu(menu_id, fl_value_lookup_string(value, kSubMenuKey));
      if (subMenu == nullptr) {
        return nullptr;
      }
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), subMenu);
    } else {
      if (strcmp(type, kCheckboxKey) == 0) {
        FlValue* checked_value = fl_value_lookup_string(value, kCheckedKey);
        if (checked_value &&
            fl_value_get_type(checked_value) == FL_VALUE_TYPE_BOOL) {
          bool checked = fl_value_get_bool(checked_value);
          gtk_check_menu_item_set_active(
              reinterpret_cast<GtkCheckMenuItem*>(menu_item), checked);
        }
      }

      FlValue* id_value = fl_value_lookup_string(value, kIdKey);
      if (id_value != nullptr &&
          fl_value_get_type(id_value) == FL_VALUE_TYPE_INT) {
        TrayCallbackData* callback_data = new TrayCallbackData();
        callback_data->menu = this;
        callback_data->menu_id = menu_id;
        callback_data->menu_item_id = fl_value_get_int(id_value);

        g_signal_connect(G_OBJECT(menu_item), "activate",
                         G_CALLBACK(Menu::menu_item_callback), callback_data);
      }
    }
  }

  return menu_item;
}