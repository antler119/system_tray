#include "tray.h"

#include <iostream>

#include <strsafe.h>
#include <windowsx.h>

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

SystemTray::SystemTray() {}

SystemTray::~SystemTray() {
  remove_tray_icon();
  destroy_icon();
  destroy_menu();
}

bool SystemTray::init_system_tray(HWND window,
                                  const std::string& title,
                                  const std::string& iconPath) {
  bool ret = false;

  do {
    tray_icon_installed_ = install_tray_icon(window, title, iconPath);

    ret = tray_icon_installed_;
  } while (false);

  return ret;
}

bool SystemTray::set_context_menu(HMENU context_menu) {
  destroy_menu();
  context_menu_ = context_menu;
  return true;
}

bool SystemTray::install_tray_icon(HWND window,
                                   const std::string& title,
                                   const std::string& iconPath) {
  bool ret = false;

  do {
    destroy_icon();

    std::wstring title_u = Utf16FromUtf8(title);
    std::wstring iconPath_u = Utf16FromUtf8(iconPath);

    icon_ = static_cast<HICON>(LoadImage(
        nullptr, iconPath_u.c_str(), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE));
    if (!icon_) {
      break;
    }

    window_ = window;

    nid_.uVersion = NOTIFYICON_VERSION_4;  // Windows Vista and later support
    nid_.hWnd = window_;
    nid_.hIcon = icon_;
    nid_.uCallbackMessage = tray_notify_callback_message_;
    StringCchCopy(nid_.szTip, _countof(nid_.szTip), title_u.c_str());
    nid_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    if (!Shell_NotifyIcon(NIM_ADD, &nid_)) {
      break;
    }

    ret = true;
  } while (false);

  return ret;
}

bool SystemTray::remove_tray_icon() {
  if (tray_icon_installed_) {
    return Shell_NotifyIcon(NIM_DELETE, &nid_);
  }
  return false;
}

bool SystemTray::reinstall_tray_icon() {
  if (tray_icon_installed_) {
    tray_icon_installed_ = Shell_NotifyIcon(NIM_ADD, &nid_);
    return tray_icon_installed_;
  }
  return false;
}

void SystemTray::destroy_icon() {
  if (icon_) {
    DestroyIcon(icon_);
    icon_ = nullptr;
  }
}

void SystemTray::destroy_menu() {
  if (context_menu_) {
    DestroyMenu(context_menu_);
    context_menu_ = nullptr;
  }
}

std::optional<LRESULT> SystemTray::HandleWindowProc(HWND hwnd,
                                                    UINT message,
                                                    WPARAM wparam,
                                                    LPARAM lparam) {
  if (message == taskbar_created_message_) {
    reinstall_tray_icon();
    return 0;
  } else if (message == tray_notify_callback_message_) {
    UINT id = HIWORD(lparam);
    UINT notifyMsg = LOWORD(lparam);
    POINT pt = {GET_X_LPARAM(wparam), GET_Y_LPARAM(wparam)};
    return OnTrayIconCallback(id, notifyMsg, pt);
  }
  return std::nullopt;
}

std::optional<LRESULT> SystemTray::OnTrayIconCallback(UINT id,
                                                      UINT notifyMsg,
                                                      const POINT& pt) {
  do {
    switch (notifyMsg) {
      case WM_LBUTTONUP: {
        ActiveWindow();
      } break;
      case WM_LBUTTONDBLCLK: {
        ActiveWindow();
      } break;
      case WM_RBUTTONUP: {
        ShowPopupMenu();
      } break;
    }

  } while (false);
  return 0;
}

void SystemTray::ShowPopupMenu() {
  if (!context_menu_) {
    return;
  }

  POINT pt{};
  GetCursorPos(&pt);

  SetForegroundWindow(window_);
  TrackPopupMenu(context_menu_, TPM_LEFTBUTTON, pt.x, pt.y, 0, window_,
                 nullptr);
  PostMessage(window_, WM_NULL, 0, 0);
}

void SystemTray::ActiveWindow() {
  if (!IsWindow(window_))
    return;

  if (!::IsWindowVisible(window_))
    ShowWindow(window_, SW_SHOW);

  if (IsIconic(window_)) {
    SendMessage(window_, WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);
  }

  BringWindowToTop(window_);
  SetForegroundWindow(window_);
}