#include "menu_manager.h"

#include "errors.h"
#include "menu.h"

constexpr char kCreateContextMenu[] = "CreateContextMenu";
constexpr char kSetLabel[] = "SetLabel";
constexpr char kSetImage[] = "SetImage";
constexpr char kSetEnable[] = "SetEnable";
constexpr char kSetCheck[] = "SetCheck";

namespace {

constexpr char kMenuIdKey[] = "menu_id";

}  // namespace

MenuManager::MenuManager(FlMethodChannel* channel) noexcept
    : channel_(channel) {}

MenuManager::~MenuManager() noexcept {
  channel_ = nullptr;
}

void MenuManager::handle_method_call(FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  // g_print("method call %s\n", method);

  if (strcmp(method, kCreateContextMenu) == 0) {
    response = create_context_menu(args);
  } else if (strcmp(method, kSetLabel) == 0) {
    response = set_label(args);
  } else if (strcmp(method, kSetImage) == 0) {
    response = set_image(args);
  } else if (strcmp(method, kSetEnable) == 0) {
    response = set_enable(args);
  } else if (strcmp(method, kSetCheck) == 0) {
    response = set_check(args);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error)) {
    g_warning("Failed to send method call response: %s", error->message);
  }
}

FlMethodResponse* MenuManager::create_context_menu(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    FlValue* menu_id_value = fl_value_lookup_string(args, kMenuIdKey);
    if (!menu_id_value ||
        fl_value_get_type(menu_id_value) != FL_VALUE_TYPE_INT) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", nullptr));
      break;
    }

    int64_t menu_id = fl_value_get_int(menu_id_value);

    std::unique_ptr<Menu> menu = std::make_unique<Menu>(channel_, menu_id);
    if (!menu) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new(errors::kOutOfMemoryError, "", nullptr));
      break;
    }

    if (!menu->create_context_menu(args)) {
      break;
    }

    add_menu(menu_id, std::move(menu));

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* MenuManager::set_label(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    std::shared_ptr<Menu> menu = get_menu(args);
    if (!menu) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new(errors::kNotFoundError, "", nullptr));
      break;
    }

    response = menu->set_label(args);

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* MenuManager::set_image(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    std::shared_ptr<Menu> menu = get_menu(args);
    if (!menu) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new(errors::kNotFoundError, "", nullptr));
      break;
    }

    response = menu->set_image(args);

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* MenuManager::set_enable(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    std::shared_ptr<Menu> menu = get_menu(args);
    if (!menu) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new(errors::kNotFoundError, "", nullptr));
      break;
    }

    response = menu->set_enable(args);

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

FlMethodResponse* MenuManager::set_check(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    std::shared_ptr<Menu> menu = get_menu(args);
    if (!menu) {
      response = FL_METHOD_RESPONSE(
          fl_method_error_response_new(errors::kNotFoundError, "", nullptr));
      break;
    }

    response = menu->set_check(args);

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }

  return response;
}

bool MenuManager::add_menu(int64_t menu_id, std::unique_ptr<Menu> menu) {
  menus_map_.emplace(menu_id, std::move(menu));
  return true;
}

std::shared_ptr<Menu> MenuManager::get_menu(int64_t menu_id) {
  auto iter = menus_map_.find(menu_id);
  return (iter != menus_map_.end()) ? iter->second : nullptr;
}

std::shared_ptr<Menu> MenuManager::get_menu(FlValue* args) {
  std::shared_ptr<Menu> menu;

  do {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
      break;
    }

    FlValue* menu_id_value = fl_value_lookup_string(args, kMenuIdKey);
    if (!menu_id_value ||
        fl_value_get_type(menu_id_value) != FL_VALUE_TYPE_INT) {
      break;
    }

    int64_t menu_id = fl_value_get_int(menu_id_value);

    menu = get_menu(menu_id);

  } while (false);

  return menu;
}