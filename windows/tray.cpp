#include "tray.h"

#include <memory>

#include <strsafe.h>
#include <windowsx.h>

#include "errors.h"
#include "menu.h"
#include "menu_manager.h"
#include "utils.h"

namespace {

constexpr wchar_t kTrayWindowClassName[] = L"FLUTTER_RUNNER_WIN32_WINDOW_TRAY";

constexpr char kChannelName[] = "flutter/system_tray/tray";

constexpr char kInitSystemTray[] = "InitSystemTray";
constexpr char kSetSystemTrayInfo[] = "SetSystemTrayInfo";
constexpr char kSetContextMenu[] = "SetContextMenu";
constexpr char kPopupContextMenu[] = "PopupContextMenu";
constexpr char kGetTitle[] = "GetTitle";
constexpr char kDestroySystemTray[] = "DestroySystemTray";

constexpr char kTitleKey[] = "title";
constexpr char kIconPathKey[] = "iconpath";
constexpr char kToolTipKey[] = "tooltip";

constexpr char kSystemTrayEventClick[] = "click";
constexpr char kSystemTrayEventRightClick[] = "right-click";
constexpr char kSystemTrayEventDoubleClick[] = "double-click";

constexpr char kSystemTrayEventCallbackMethod[] = "SystemTrayEventCallback";

}  // namespace

Tray::Tray(flutter::PluginRegistrarWindows* registrar,
           std::weak_ptr<MenuManager> menu_manager) noexcept
    : registrar_(registrar), menu_manager_(menu_manager) {
  assert(registrar_);

  window_proc_id_ = registrar_->RegisterTopLevelWindowProcDelegate(
      [this](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        return HandleWindowProc(hwnd, message, wparam, lparam);
      });

  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar_->messenger(), kChannelName,
          &flutter::StandardMethodCodec::GetInstance());

  channel->SetMethodCallHandler([this](const auto& call, auto result) {
    HandleMethodCall(call, std::move(result));
  });

  channel_ = std::move(channel);
}

Tray::~Tray() noexcept {
  registrar_->UnregisterTopLevelWindowProcDelegate(window_proc_id_);

  destroyTray();

  DestoryTrayWindow();
  UnregisterWindowClass();

  registrar_ = nullptr;
}

void Tray::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  // printf("method call %s\n", method_call.method_name().c_str());

  if (method_call.method_name().compare(kInitSystemTray) == 0) {
    initTray(method_call, *result);
  } else if (method_call.method_name().compare(kSetSystemTrayInfo) == 0) {
    setTrayInfo(method_call, *result);
  } else if (method_call.method_name().compare(kSetContextMenu) == 0) {
    setContextMenu(method_call, *result);
  } else if (method_call.method_name().compare(kPopupContextMenu) == 0) {
    popupContextMenu(method_call, *result);
  } else if (method_call.method_name().compare(kGetTitle) == 0) {
    getTitle(method_call, *result);
  } else if (method_call.method_name().compare(kDestroySystemTray) == 0) {
    destroyTray(method_call, *result);
  } else {
    result->NotImplemented();
  }
}

void Tray::initTray(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    flutter::FlutterView* view = registrar_->GetView();
    if (!view) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* map =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!map) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const std::string* title =
        std::get_if<std::string>(utils::ValueOrNull(*map, kTitleKey));

    const std::string* iconPath =
        std::get_if<std::string>(utils::ValueOrNull(*map, kIconPathKey));

    const std::string* toolTip =
        std::get_if<std::string>(utils::ValueOrNull(*map, kToolTipKey));

    HWND window = GetAncestor(view->GetNativeWindow(), GA_ROOT);
    if (!initTray(window, title, iconPath, toolTip)) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    result.Success(flutter::EncodableValue(true));

  } while (false);
}

void Tray::setTrayInfo(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* map =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!map) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const std::string* title =
        std::get_if<std::string>(utils::ValueOrNull(*map, kTitleKey));

    const std::string* iconPath =
        std::get_if<std::string>(utils::ValueOrNull(*map, kIconPathKey));

    const std::string* toolTip =
        std::get_if<std::string>(utils::ValueOrNull(*map, kToolTipKey));

    if (!setTrayInfo(title, iconPath, toolTip)) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    result.Success(flutter::EncodableValue(true));

  } while (false);
}

void Tray::setContextMenu(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* menu_id = std::get_if<int>(method_call.arguments());
    if (!menu_id) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    SetContextMenuId(*menu_id);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void Tray::popupContextMenu(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    ShowPopupMenu();

    result.Success(flutter::EncodableValue(true));
  } while (false);
}

void Tray::getTitle(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  result.Success(flutter::EncodableValue(""));
}

void Tray::destroyTray(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  destroyTray();
  result.Success(flutter::EncodableValue(true));
}

bool Tray::initTray(HWND window,
                    const std::string* title,
                    const std::string* iconPath,
                    const std::string* toolTip) {
  bool ret = false;

  do {
    if (tray_icon_installed_) {
      ret = true;
      break;
    }

    if (tray_window_ == nullptr) {
      if (!CreateTrayWindow()) {
        break;
      }
    }

    tray_icon_installed_ = installTrayIcon(window, title, iconPath, toolTip);

    ret = tray_icon_installed_;
  } while (false);

  return ret;
}

bool Tray::setTrayInfo(const std::string* title,
                       const std::string* iconPath,
                       const std::string* toolTip) {
  bool ret = false;

  do {
    if (!IsWindow(window_)) {
      break;
    }

    if (!tray_icon_installed_) {
      break;
    }

    if (toolTip) {
      nid_.uFlags |= NIF_TIP;
      std::wstring toolTip_u = utils::Utf16FromUtf8(*toolTip);
      StringCchCopy(nid_.szTip, _countof(nid_.szTip), toolTip_u.c_str());
    }

    if (iconPath) {
      destroyIcon();

      nid_.uFlags |= NIF_ICON;
      std::wstring iconPath_u = utils::Utf16FromUtf8(*iconPath);
      icon_ =
          static_cast<HICON>(LoadImage(nullptr, iconPath_u.c_str(), IMAGE_ICON,
                                       0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE));
      nid_.hIcon = icon_;
    }

    if (!Shell_NotifyIcon(NIM_MODIFY, &nid_)) {
      break;
    }

    ret = true;
  } while (false);

  return ret;
}

bool Tray::installTrayIcon(HWND window,
                           const std::string* title,
                           const std::string* iconPath,
                           const std::string* toolTip) {
  bool ret = false;

  do {
    destroyIcon();

    std::wstring title_u = title ? utils::Utf16FromUtf8(*title) : L"";
    std::wstring iconPath_u = iconPath ? utils::Utf16FromUtf8(*iconPath) : L"";
    std::wstring toolTip_u = toolTip ? utils::Utf16FromUtf8(*toolTip) : L"";

    icon_ =
        static_cast<HICON>(LoadImage(nullptr, iconPath_u.c_str(), IMAGE_ICON, 0,
                                     0, LR_LOADFROMFILE | LR_DEFAULTSIZE));
    if (!icon_) {
      break;
    }

    window_ = window;

    nid_.cbSize = {sizeof(NOTIFYICONDATA)};
    nid_.uVersion = NOTIFYICON_VERSION_4;  // Windows Vista and later support
    nid_.hWnd = window_;
    nid_.hIcon = icon_;
    nid_.uCallbackMessage = tray_notify_callback_message_;
    StringCchCopy(nid_.szTip, _countof(nid_.szTip), toolTip_u.c_str());
    nid_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    if (!Shell_NotifyIcon(NIM_ADD, &nid_)) {
      break;
    }

    ret = true;
  } while (false);

  return ret;
}

bool Tray::removeTrayIcon() {
  if (tray_icon_installed_) {
    if (Shell_NotifyIcon(NIM_DELETE, &nid_)) {
      tray_icon_installed_ = false;
      memset(&nid_, 0, sizeof(nid_));
      return true;
    }
  }
  return false;
}

void Tray::destroyTray() {
  context_menu_id_ = -1;
  removeTrayIcon();
  destroyIcon();
}

bool Tray::reinstallTrayIcon() {
  if (tray_icon_installed_) {
    tray_icon_installed_ = Shell_NotifyIcon(NIM_ADD, &nid_);
    return tray_icon_installed_;
  }
  return false;
}

void Tray::destroyIcon() {
  if (icon_) {
    DestroyIcon(icon_);
    icon_ = nullptr;
  }
}

std::optional<LRESULT> Tray::HandleWindowProc(HWND hwnd,
                                              UINT message,
                                              WPARAM wparam,
                                              LPARAM lparam) {
  if (message == taskbar_created_message_) {
    reinstallTrayIcon();
    return 0;
  } else if (message == tray_notify_callback_message_) {
    UINT id = HIWORD(lparam);
    UINT notifyMsg = LOWORD(lparam);
    POINT pt = {GET_X_LPARAM(wparam), GET_Y_LPARAM(wparam)};
    return OnTrayIconCallback(id, notifyMsg, pt);
  }
  return std::nullopt;
}

std::optional<LRESULT> Tray::OnTrayIconCallback(UINT id,
                                                UINT notifyMsg,
                                                const POINT& pt) {
  do {
    switch (notifyMsg) {
      case WM_LBUTTONDOWN: {
      } break;
      case WM_LBUTTONUP: {
        OnSystemTrayEventCallback(kSystemTrayEventClick);
      } break;
      case WM_LBUTTONDBLCLK: {
        OnSystemTrayEventCallback(kSystemTrayEventDoubleClick);
      } break;
      case WM_RBUTTONDOWN: {
      } break;
      case WM_RBUTTONUP: {
        OnSystemTrayEventCallback(kSystemTrayEventRightClick);
      } break;
    }

  } while (false);
  return 0;
}

const wchar_t* Tray::GetTrayWindowClass() {
  if (!tray_class_registered_) {
    WNDCLASS window_class{};
    window_class.hCursor = nullptr;
    window_class.lpszClassName = kTrayWindowClassName;
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = GetModuleHandle(nullptr);
    window_class.hIcon = nullptr;
    window_class.hbrBackground = 0;
    window_class.lpszMenuName = nullptr;
    window_class.lpfnWndProc = Tray::TrayWndProc;
    RegisterClass(&window_class);
    tray_class_registered_ = true;
  }

  return kTrayWindowClassName;
}

void Tray::UnregisterWindowClass() {
  UnregisterClass(kTrayWindowClassName, nullptr);
  tray_class_registered_ = false;
}

LRESULT CALLBACK Tray::TrayWndProc(HWND const window,
                                   UINT const message,
                                   WPARAM const wparam,
                                   LPARAM const lparam) noexcept {
  if (message == WM_NCCREATE) {
    auto window_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
    SetWindowLongPtr(window, GWLP_USERDATA,
                     reinterpret_cast<LONG_PTR>(window_struct->lpCreateParams));
  } else if (Tray* that = GetThisFromHandle(window)) {
    return that->TrayMessageHandler(window, message, wparam, lparam);
  }

  return DefWindowProc(window, message, wparam, lparam);
}

Tray* Tray::GetThisFromHandle(HWND const window) noexcept {
  return reinterpret_cast<Tray*>(GetWindowLongPtr(window, GWLP_USERDATA));
}

LRESULT Tray::TrayMessageHandler(HWND window,
                                 UINT const message,
                                 WPARAM const wparam,
                                 LPARAM const lparam) noexcept {
  return DefWindowProc(window, message, wparam, lparam);
}

bool Tray::CreateTrayWindow() {
  HWND window =
      CreateWindow(GetTrayWindowClass(), nullptr, WS_OVERLAPPEDWINDOW, -1, -1,
                   0, 0, nullptr, nullptr, GetModuleHandle(nullptr), this);
  if (!window) {
    return false;
  }

  tray_window_ = window;
  return true;
}

void Tray::DestoryTrayWindow() {
  if (tray_window_) {
    DestroyWindow(tray_window_);
    tray_window_ = nullptr;
  }
}

void Tray::OnSystemTrayEventCallback(const std::string& eventName) {
  channel_->InvokeMethod(kSystemTrayEventCallbackMethod,
                         std::make_unique<flutter::EncodableValue>(eventName));
}

void Tray::ShowPopupMenu() {
  if (menu_manager_.expired()) {
    return;
  }

  std::shared_ptr<MenuManager> menu_manager = menu_manager_.lock();
  std::shared_ptr<Menu> menu = menu_manager->GetMenu(GetContextMenuId());
  if (!menu) {
    return;
  }

  POINT pt{};
  GetCursorPos(&pt);

  SetForegroundWindow(tray_window_);

  menu->PopupContextMenu(window_, pt);

  PostMessage(window_, WM_NULL, 0, 0);
}

void Tray::SetContextMenuId(int context_menu_id) {
  context_menu_id_ = context_menu_id;
}

int Tray::GetContextMenuId() const {
  return context_menu_id_;
}