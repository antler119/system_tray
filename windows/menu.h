#ifndef __MENU_H__
#define __MENU_H__

#include <memory>
#include <string>

#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

class Menu {
 public:
  Menu(std::weak_ptr<flutter::MethodChannel<>> channel, int menu_id) noexcept;
  ~Menu() noexcept;

  bool CreateContextMenu(
      const flutter::MethodCall<flutter::EncodableValue>& method_call);

  void SetLabel(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                flutter::MethodResult<flutter::EncodableValue>& result);

  void SetImage(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                flutter::MethodResult<flutter::EncodableValue>& result);

  void SetEnable(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      flutter::MethodResult<flutter::EncodableValue>& result);

  void SetCheck(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                flutter::MethodResult<flutter::EncodableValue>& result);

  HMENU GetMenu() const;

  void PopupContextMenu(HWND window, const POINT& pt);

 protected:
  bool ValueToMenu(HMENU menu, const flutter::EncodableList& representation);
  bool ValueToMenuItem(HMENU menu, const flutter::EncodableMap& representation);

  bool CreateContextMenu(const flutter::EncodableList& representation);
  void DestroyContextMenu();

  void SetLabel(int menu_item_id, const std::string& label);
  void SetImage(int menu_item_id, const std::string& image);
  void SetEnable(int menu_item_id, bool enabled);
  void SetCheck(int menu_item_id, bool checked);

  int MenuId() const;

 protected:
  std::weak_ptr<flutter::MethodChannel<>> channel_;

  int menu_id_ = -1;

  HMENU menu_ = nullptr;
};

#endif  // __MENU_H__