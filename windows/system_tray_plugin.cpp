#include "include/system_tray/system_tray_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

#include "tray.h"

namespace {

const static char kBadArgumentsError[] = "Bad Arguments";
const static char kMenuConstructionError[] = "Menu Construction Error";

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

// Starting point for the generated menu IDs.
const unsigned int kFirstMenuId = 1000;

// Converts the given UTF-8 string to UTF-16.
static std::wstring Utf16FromUtf8(const std::string& utf8_string) {
  if (utf8_string.empty()) {
    return std::wstring();
  }
  int target_length =
      ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_string.data(),
                            static_cast<int>(utf8_string.length()), nullptr, 0);
  if (target_length == 0) {
    return std::wstring();
  }
  std::wstring utf16_string;
  utf16_string.resize(target_length);
  int converted_length =
      ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_string.data(),
                            static_cast<int>(utf8_string.length()),
                            utf16_string.data(), target_length);
  if (converted_length == 0) {
    return std::wstring();
  }
  return utf16_string;
}

const flutter::EncodableValue* ValueOrNull(const flutter::EncodableMap& map,
                                           const char* key) {
  auto it = map.find(flutter::EncodableValue(key));
  if (it == map.end()) {
    return nullptr;
  }
  return &(it->second);
}

class SystemTrayPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

  SystemTrayPlugin(flutter::PluginRegistrarWindows* registrar,
                   std::unique_ptr<flutter::MethodChannel<>> channel);

  virtual ~SystemTrayPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

 protected:
  // Called for top-level WindowProc delegation.
  std::optional<LRESULT> HandleWindowProc(HWND hwnd,
                                          UINT message,
                                          WPARAM wparam,
                                          LPARAM lparam);

  void init_system_tray(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void set_system_tray_info(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void set_context_menu(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  bool value_to_menu(HMENU menu, const flutter::EncodableList& representation);
  bool value_to_menu_item(HMENU menu,
                          const flutter::EncodableMap& representation);

 protected:
  // The registrar for this plugin.
  flutter::PluginRegistrarWindows* registrar_ = nullptr;

  // The cannel to send menu item activations on.
  std::unique_ptr<flutter::MethodChannel<>> channel_;

  // The ID of the registered WindowProc handler.
  int window_proc_id_;

  std::unique_ptr<SystemTray> system_tray_;
};

// static
void SystemTrayPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows* registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), kChannelName,
          &flutter::StandardMethodCodec::GetInstance());

  auto* channel_pointer = channel.get();

  auto plugin =
      std::make_unique<SystemTrayPlugin>(registrar, std::move(channel));

  channel_pointer->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto& call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

SystemTrayPlugin::SystemTrayPlugin(
    flutter::PluginRegistrarWindows* registrar,
    std::unique_ptr<flutter::MethodChannel<>> channel)
    : registrar_(registrar), channel_(std::move(channel)) {
  window_proc_id_ = registrar_->RegisterTopLevelWindowProcDelegate(
      [this](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        return HandleWindowProc(hwnd, message, wparam, lparam);
      });

  system_tray_ = std::make_unique<SystemTray>();
}

SystemTrayPlugin::~SystemTrayPlugin() {
  registrar_->UnregisterTopLevelWindowProcDelegate(window_proc_id_);
}

void SystemTrayPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  printf("method call %s\n", method_call.method_name().c_str());

  if (method_call.method_name().compare(kInitSystemTray) == 0) {
    init_system_tray(method_call, *result);
  } else if (method_call.method_name().compare(kSetSystemTrayInfo) == 0) {
    set_system_tray_info(method_call, *result);
  } else if (method_call.method_name().compare(kSetContextMenu) == 0) {
    set_context_menu(method_call, *result);
  } else {
    result->NotImplemented();
  }
}

void SystemTrayPlugin::init_system_tray(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    flutter::FlutterView* view = registrar_->GetView();
    HWND window =
        view ? GetAncestor(view->GetNativeWindow(), GA_ROOT) : nullptr;
    if (!view) {
      result.Error(kBadArgumentsError, "Expected window",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* map =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!map) {
      result.Error(kBadArgumentsError, "Expected map",
                   flutter::EncodableValue(false));
      break;
    }

    const std::string* title =
        std::get_if<std::string>(ValueOrNull(*map, kTitleKey));
    if (!title) {
      result.Error(kBadArgumentsError, "Unable to get title",
                   flutter::EncodableValue(false));
      break;
    }

    const std::string* iconPath =
        std::get_if<std::string>(ValueOrNull(*map, kIconPathKey));
    if (!iconPath) {
      result.Error(kBadArgumentsError, "Unable to get icon path",
                   flutter::EncodableValue(false));
      break;
    }

    const std::string* toolTip =
        std::get_if<std::string>(ValueOrNull(*map, kToolTipKey));
    if (!toolTip) {
      result.Error(kBadArgumentsError, "Unable to get tooltip",
                   flutter::EncodableValue(false));
      break;
    }

    if (!system_tray_->init_system_tray(window, *title, *iconPath, *toolTip)) {
      result.Error(kBadArgumentsError, "Unable to init system tray",
                   flutter::EncodableValue(false));
      break;
    }

    result.Success(flutter::EncodableValue(true));

  } while (false);
}

void SystemTrayPlugin::set_system_tray_info(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* map =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!map) {
      result.Error(kBadArgumentsError, "Expected map",
                   flutter::EncodableValue(false));
      break;
    }

    const std::string* title =
        std::get_if<std::string>(ValueOrNull(*map, kTitleKey));

    const std::string* iconPath =
        std::get_if<std::string>(ValueOrNull(*map, kIconPathKey));

    const std::string* toolTip =
        std::get_if<std::string>(ValueOrNull(*map, kToolTipKey));

    if (!system_tray_->set_system_tray_info(title, iconPath, toolTip)) {
      result.Error(kBadArgumentsError, "Unable to set system tray info",
                   flutter::EncodableValue(false));
      break;
    }

    result.Success(flutter::EncodableValue(true));

  } while (false);
}

bool SystemTrayPlugin::value_to_menu(
    HMENU menu,
    const flutter::EncodableList& representation) {
  for (const auto& item : representation) {
    if (!value_to_menu_item(menu, std::get<flutter::EncodableMap>(item))) {
      return false;
    }
  }
  return true;
}

bool SystemTrayPlugin::value_to_menu_item(
    HMENU menu,
    const flutter::EncodableMap& representation) {
  const auto* type =
      std::get_if<std::string>(ValueOrNull(representation, kTypeKey));
  if (!type) {
    return false;
  }

  if (type->compare(kSeparatorKey) == 0) {
    AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
  } else {
    UINT flags = MF_STRING;

    const auto* label =
        std::get_if<std::string>(ValueOrNull(representation, kLabelKey));
    std::wstring label_u(label ? Utf16FromUtf8(*label) : L"");

    const auto* enabled =
        std::get_if<bool>(ValueOrNull(representation, kEnabledKey));

    flags |= (enabled == nullptr || *enabled) ? MF_ENABLED : MF_GRAYED;

    UINT_PTR item_id = 0;
    if (type->compare(kSubMenuKey) == 0) {
      flags |= MF_POPUP;

      const auto* children = std::get_if<flutter::EncodableList>(
          ValueOrNull(representation, kSubMenuKey));
      if (children) {
        HMENU submenu = ::CreatePopupMenu();
        if (value_to_menu(submenu, *children)) {
          item_id = reinterpret_cast<UINT_PTR>(submenu);
        } else {
          DestroyMenu(submenu);
        }
      }
    } else {
      const auto* menu_id =
          std::get_if<int32_t>(ValueOrNull(representation, kIdKey));
      item_id = menu_id ? (kFirstMenuId + *menu_id) : 0;
    }

    AppendMenu(menu, flags, item_id, label_u.c_str());
  }

  return true;
}

void SystemTrayPlugin::set_context_menu(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  HMENU popup_menu = nullptr;

  do {
    const auto* list =
        std::get_if<flutter::EncodableList>(method_call.arguments());
    if (!list) {
      result.Error(kBadArgumentsError, "Expected list",
                   flutter::EncodableValue(false));
      break;
    }

    popup_menu = CreatePopupMenu();
    if (!value_to_menu(popup_menu, *list)) {
      result.Error(kBadArgumentsError, "Unable to contruct menu",
                   flutter::EncodableValue(false));
      break;
    }

    if (!system_tray_->set_context_menu(popup_menu)) {
      result.Error(kBadArgumentsError, "Unable to set context menu",
                   flutter::EncodableValue(false));
      break;
    }

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);

  if (popup_menu) {
    DestroyMenu(popup_menu);
  }
}

std::optional<LRESULT> SystemTrayPlugin::HandleWindowProc(HWND hwnd,
                                                          UINT message,
                                                          WPARAM wparam,
                                                          LPARAM lparam) {
  if (message == WM_COMMAND) {
    DWORD menu_id = LOWORD(wparam);
    if (menu_id >= kFirstMenuId) {
      int32_t flutter_id = menu_id - kFirstMenuId;
      channel_->InvokeMethod(
          kMenuItemSelectedCallbackMethod,
          std::make_unique<flutter::EncodableValue>(flutter_id));
      return 0;
    }
  } else {
    if (system_tray_) {
      return system_tray_->HandleWindowProc(hwnd, message, wparam, lparam);
    }
  }
  return std::nullopt;
}

}  // namespace

void SystemTrayPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  SystemTrayPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
