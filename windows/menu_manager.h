#ifndef __MENU_MANAGER_H__
#define __MENU_MANAGER_H__

#include <memory>
#include <optional>
#include <unordered_map>

#include <Windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

class Menu;

class MenuManager {
 public:
  MenuManager(flutter::PluginRegistrarWindows* registrar) noexcept;
  ~MenuManager() noexcept;

  std::shared_ptr<Menu> GetMenu(int menu_id);

 protected:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  void CreateContextMenu(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void SetLabel(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                flutter::MethodResult<flutter::EncodableValue>& result);

  void SetImage(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                flutter::MethodResult<flutter::EncodableValue>& result);

  void SetEnable(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void SetCheck(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                flutter::MethodResult<flutter::EncodableValue>& result);

  bool AddMenu(int menu_id, std::unique_ptr<Menu> menu);
  std::shared_ptr<Menu> GetMenu(
      const flutter::MethodCall<flutter::EncodableValue>& method_call);

 protected:
  flutter::PluginRegistrarWindows* registrar_ = nullptr;
  std::shared_ptr<flutter::MethodChannel<>> channel_;

  std::unordered_map<int, std::shared_ptr<Menu>> menus_map_;
};

#endif  // __MENU_MANAGER_H__