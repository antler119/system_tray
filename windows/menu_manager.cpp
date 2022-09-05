#include "menu_manager.h"

#include <memory>

#include "errors.h"
#include "menu.h"
#include "utils.h"

namespace {

constexpr char kChannelName[] = "flutter/system_tray/menu_manager";

constexpr char kCreateContextMenu[] = "CreateContextMenu";
constexpr char kSetLabel[] = "SetLabel";
constexpr char kSetImage[] = "SetImage";
constexpr char kSetEnable[] = "SetEnable";
constexpr char kSetCheck[] = "SetCheck";

constexpr char kMenuIdKey[] = "menu_id";

}  // namespace

MenuManager::MenuManager(flutter::PluginRegistrarWindows* registrar) noexcept
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
  // printf("method call %s\n", method_call.method_name().c_str());

  if (method_call.method_name().compare(kCreateContextMenu) == 0) {
    CreateContextMenu(method_call, *result);
  } else if (method_call.method_name().compare(kSetLabel) == 0) {
    SetLabel(method_call, *result);
  } else if (method_call.method_name().compare(kSetImage) == 0) {
    SetImage(method_call, *result);
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
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuIdKey));
    if (!menu_id) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    std::unique_ptr<Menu> menu = std::make_unique<Menu>(channel_, *menu_id);
    if (!menu) {
      result.Error(errors::kOutOfMemoryError, "",
                   flutter::EncodableValue(false));
      break;
    }

    if (!menu->CreateContextMenu(method_call)) {
      result.Error(errors::kFailedError, "", flutter::EncodableValue(false));
      break;
    }

    AddMenu(*menu_id, std::move(menu));

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void MenuManager::SetLabel(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    std::shared_ptr<Menu> menu = GetMenu(method_call);
    if (!menu) {
      result.Error(errors::kNotFoundError, "", flutter::EncodableValue(false));
      break;
    }

    menu->SetLabel(method_call, result);
    return;

  } while (false);
}

void MenuManager::SetImage(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    std::shared_ptr<Menu> menu = GetMenu(method_call);
    if (!menu) {
      result.Error(errors::kNotFoundError, "", flutter::EncodableValue(false));
      break;
    }

    menu->SetImage(method_call, result);
    return;

  } while (false);
}

void MenuManager::SetEnable(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    std::shared_ptr<Menu> menu = GetMenu(method_call);
    if (!menu) {
      result.Error(errors::kNotFoundError, "", flutter::EncodableValue(false));
      break;
    }

    menu->SetEnable(method_call, result);
    return;

  } while (false);
}

void MenuManager::SetCheck(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    std::shared_ptr<Menu> menu = GetMenu(method_call);
    if (!menu) {
      result.Error(errors::kNotFoundError, "", flutter::EncodableValue(false));
      break;
    }

    menu->SetCheck(method_call, result);
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

std::shared_ptr<Menu> MenuManager::GetMenu(
    const flutter::MethodCall<flutter::EncodableValue>& method_call) {
  std::shared_ptr<Menu> menu;

  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      break;
    }

    const auto* menu_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuIdKey));
    if (!menu_id) {
      break;
    }

    menu = GetMenu(*menu_id);

  } while (false);

  return menu;
}
