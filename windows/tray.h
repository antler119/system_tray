#ifndef __Tray_H__
#define __Tray_H__

#include <optional>
#include <string>

#include <windows.h>

#include <shellapi.h>

class SystemTray {
 public:
  SystemTray();
  ~SystemTray();

  bool init_system_tray(HWND window,
                        const std::string* title,
                        const std::string* iconPath,
                        const std::string* toolTip);

  bool set_system_tray_info(const std::string* title,
                            const std::string* iconPath,
                            const std::string* toolTip);

  bool set_context_menu(HMENU context_menu);

  std::optional<LRESULT> HandleWindowProc(HWND hwnd,
                                          UINT message,
                                          WPARAM wparam,
                                          LPARAM lparam);

 protected:
  bool install_tray_icon(HWND window,
                         const std::string* title,
                         const std::string* iconPath,
                         const std::string* toolTip);
  bool remove_tray_icon();
  bool reinstall_tray_icon();
  void destroy_icon();
  void destroy_menu();

  std::optional<LRESULT> OnTrayIconCallback(UINT id,
                                            UINT notifyMsg,
                                            const POINT& pt);

  void ShowPopupMenu();
  void ActiveWindow();

 protected:
  HWND window_ = nullptr;
  HMENU context_menu_ = nullptr;
  HICON icon_ = nullptr;

  UINT taskbar_created_message_ = RegisterWindowMessage(L"TaskbarCreated");
  UINT tray_notify_callback_message_ =
      RegisterWindowMessage(L"SystemTrayNotify");
  NOTIFYICONDATA nid_ = {sizeof(NOTIFYICONDATA)};
  bool tray_icon_installed_ = false;
};

#endif