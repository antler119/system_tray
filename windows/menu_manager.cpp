#include "menu_manager.h"

#include <memory>

#include "errors.h"
#include "menu.h"
#include "utils.h"

namespace {

constexpr char kChannelName[] = "flutter/system_tray/menu_manager";

constexpr char kCreateContextMenu[] = "createContextMenu";
constexpr char kSetLable[] = "setLable";
constexpr char kSetEnable[] = "setEnable";
constexpr char kSetCheck[] = "setCheck";

constexpr char kMenuIdKey[] = "menu_id";
constexpr char kMenuItemIdKey[] = "menu_item_id";
constexpr char kMenuListKey[] = "menu_list";
constexpr char kLabelKey[] = "label";
constexpr char kImageKey[] = "image";
constexpr char kEnabledKey[] = "enabled";
constexpr char kCheckedKey[] = "checked";

constexpr char kMenuItemSelectedCallbackMethod[] = "MenuItemSelectedCallback";

}  // namespace

MenuManager::MenuManager(flutter::PluginRegistrarWindows* registrar)
    : registrar_(registrar) {
  assert(registrar_);

  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar_->messenger(), kChannelName,
          &flutter::StandardMethodCodec::GetInstance());

  channel->SetMethodCallHandler([this](const auto& call, auto result) {
    HandleMethodCall(call, std::move(result));
  });

  channel_ = std::move(channel);
}

MenuManager::~MenuManager() noexcept {
  registrar_ = nullptr;
}

void MenuManager::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  printf("method call %s\n", method_call.method_name().c_str());

  if (method_call.method_name().compare(kCreateContextMenu) == 0) {
    CreateContextMenu(method_call, *result);
  } else if (method_call.method_name().compare(kSetLable) == 0) {
    SetLable(method_call, *result);
  } else if (method_call.method_name().compare(kSetEnable) == 0) {
    SetEnable(method_call, *result);
  } else if (method_call.method_name().compare(kSetCheck) == 0) {
    SetCheck(method_call, *result);
  } else {
    result->NotImplemented();
  }
}

void MenuManager::CreateContextMenu(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "Expected map",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuIdKey));
    if (!menu_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* list = std::get_if<flutter::EncodableList>(
        utils::ValueOrNull(*params, kMenuListKey));
    if (!list) {
      result.Error(errors::kBadArgumentsError, "Expected list",
                   flutter::EncodableValue(false));
      break;
    }

    std::unique_ptr<Menu> menu = std::make_unique<Menu>(channel_, *menu_id);
    if (!menu) {
      result.Error(errors::kOutOfMemoryError, "",
                   flutter::EncodableValue(false));
      break;
    }

    if (!menu->CreateContextMenu(*list)) {
      result.Error(errors::kFailedError, "", flutter::EncodableValue(false));
      break;
    }

    AddMenu(*menu_id, std::move(menu));

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void MenuManager::SetLable(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "Expected map",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuIdKey));
    if (!menu_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_item_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuItemIdKey));
    if (!menu_item_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu item id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* label =
        std::get_if<std::string>(utils::ValueOrNull(*params, kLabelKey));
    if (!label) {
      result.Error(errors::kBadArgumentsError, "Expected string",
                   flutter::EncodableValue(false));
      break;
    }

    std::shared_ptr<Menu> menu = GetMenu(*menu_id);
    if (!menu) {
      result.Error(errors::kNotFoundError, "", flutter::EncodableValue(false));
      break;
    }

    menu->SetLable(*menu_item_id, *label);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void MenuManager::SetImage(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "Expected map",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuIdKey));
    if (!menu_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_item_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuItemIdKey));
    if (!menu_item_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu item id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* image =
        std::get_if<std::string>(utils::ValueOrNull(*params, kImageKey));
    if (!image) {
      result.Error(errors::kBadArgumentsError, "Expected string",
                   flutter::EncodableValue(false));
      break;
    }

    std::shared_ptr<Menu> menu = GetMenu(*menu_id);
    if (!menu) {
      result.Error(errors::kNotFoundError, "", flutter::EncodableValue(false));
      break;
    }

    menu->SetImage(*menu_item_id, *image);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void MenuManager::SetEnable(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "Expected map",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuIdKey));
    if (!menu_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_item_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuItemIdKey));
    if (!menu_item_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu item id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* enabled =
        std::get_if<bool>(utils::ValueOrNull(*params, kEnabledKey));
    if (!enabled) {
      result.Error(errors::kBadArgumentsError, "Expected bool",
                   flutter::EncodableValue(false));
      break;
    }

    std::shared_ptr<Menu> menu = GetMenu(*menu_id);
    if (!menu) {
      result.Error(errors::kNotFoundError, "", flutter::EncodableValue(false));
      break;
    }

    menu->SetEnable(*menu_item_id, *enabled);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void MenuManager::SetCheck(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "Expected map",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuIdKey));
    if (!menu_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_item_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuItemIdKey));
    if (!menu_item_id) {
      result.Error(errors::kBadArgumentsError, "Expected menu item id",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* checked =
        std::get_if<bool>(utils::ValueOrNull(*params, kCheckedKey));
    if (!checked) {
      result.Error(errors::kBadArgumentsError, "Expected bool",
                   flutter::EncodableValue(false));
      break;
    }

    std::shared_ptr<Menu> menu = GetMenu(*menu_id);
    if (!menu) {
      result.Error(errors::kNotFoundError, "", flutter::EncodableValue(false));
      break;
    }

    menu->SetCheck(*menu_item_id, *checked);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

bool MenuManager::AddMenu(int menu_id, std::unique_ptr<Menu> menu) {
  menus_map_.emplace(menu_id, std::move(menu));
  return true;
}

std::shared_ptr<Menu> MenuManager::GetMenu(int menu_id) {
  auto iter = menus_map_.find(menu_id);
  return (iter != menus_map_.end()) ? iter->second : nullptr;
}