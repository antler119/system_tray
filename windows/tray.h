#ifndef __TRAY_H__
#define __TRAY_H__

#include <memory>
#include <optional>
#include <string>

#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

class MenuManager;

class Tray {
 public:
  Tray(flutter::PluginRegistrarWindows* registrar,
       std::weak_ptr<MenuManager> menu_manager) noexcept;
  ~Tray() noexcept;

 protected:
  std::optional<LRESULT> HandleWindowProc(HWND hwnd,
                                          UINT message,
                                          WPARAM wparam,
                                          LPARAM lparam);

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  void initTray(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                flutter::MethodResult<flutter::EncodableValue>& result);

  void setTrayInfo(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void setContextMenu(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void popupContextMenu(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void getTitle(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                flutter::MethodResult<flutter::EncodableValue>& result);

  void destroyTray(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  bool initTray(HWND window,
                const std::string* title,
                const std::string* iconPath,
                const std::string* toolTip);

  bool setTrayInfo(const std::string* title,
                   const std::string* iconPath,
                   const std::string* toolTip);

  bool installTrayIcon(HWND window,
                       const std::string* title,
                       const std::string* iconPath,
                       const std::string* toolTip);
  void destroyTray();
  bool removeTrayIcon();
  bool reinstallTrayIcon();
  void destroyIcon();

  std::optional<LRESULT> OnTrayIconCallback(UINT id,
                                            UINT notifyMsg,
                                            const POINT& pt);

  const wchar_t* GetTrayWindowClass();
  void UnregisterWindowClass();

  static LRESULT CALLBACK TrayWndProc(HWND const window,
                                      UINT const message,
                                      WPARAM const wparam,
                                      LPARAM const lparam) noexcept;

  static Tray* GetThisFromHandle(HWND const window) noexcept;

  virtual LRESULT TrayMessageHandler(HWND window,
                                     UINT const message,
                                     WPARAM const wparam,
                                     LPARAM const lparam) noexcept;

  bool CreateTrayWindow();
  void DestoryTrayWindow();

  void OnSystemTrayEventCallback(const std::string& eventName);

  void ShowPopupMenu();

  void SetContextMenuId(int context_menu_id);
  int GetContextMenuId() const;

 protected:
  flutter::PluginRegistrarWindows* registrar_ = nullptr;
  std::unique_ptr<flutter::MethodChannel<>> channel_;

  // The ID of the registered WindowProc handler.
  int window_proc_id_ = -1;

  HWND tray_window_ = nullptr;
  bool tray_class_registered_ = false;
  HWND window_ = nullptr;
  HICON icon_ = nullptr;

  UINT taskbar_created_message_ = RegisterWindowMessage(L"TaskbarCreated");

  UINT tray_notify_callback_message_ =
      RegisterWindowMessage(L"SystemTrayNotify");

  NOTIFYICONDATA nid_ = {sizeof(NOTIFYICONDATA)};

  bool tray_icon_installed_ = false;

  std::weak_ptr<MenuManager> menu_manager_;

  int context_menu_id_ = -1;
};

#endif  // __TRAY_H__