#ifndef __APPWINDOW_H__
#define __APPWINDOW_H__

#include <memory>

#include <Windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

class AppWindow {
 public:
  AppWindow(flutter::PluginRegistrarWindows* registrar) noexcept;
  ~AppWindow() noexcept;

 protected:
  void initAppWindow(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void showAppWindow(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void hideAppWindow(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void closeAppWindow(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void destroyAppWindow(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);
    
  bool initAppWindow(HWND window, HWND flutter_window);
  bool showAppWindow(bool visible);
  bool closeAppWindow();
  bool destroyAppWindow();

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  void activeWindow();
  void refreshFlutterWindow();

 protected:
  flutter::PluginRegistrarWindows* registrar_ = nullptr;
  std::unique_ptr<flutter::MethodChannel<>> channel_;

  HWND window_ = nullptr;
  HWND flutter_window_ = nullptr;
};

#endif  // __APPWINDOW_H__