import 'dart:async';

import 'menu.dart';
import 'platform/system_tray_platform_interface.dart';

/// A callback provided to [SystemTray] to handle system tray click event.
typedef SystemTrayEventCallback = void Function(String eventName);

/// Representation of system tray
class SystemTray {
  SystemTray() {
    SystemTrayPlatform.instance.onSystemTrayEvent = _callbackHandler;
  }

  ///
  SystemTrayEventCallback? _systemTrayEventCallback;

  /// Show a SystemTray icon
  Future<bool> initSystemTray({
    required String iconPath,
    String? title,
    String? toolTip,
    bool isTemplate = false,
  }) async {
    return await SystemTrayPlatform.instance.initSystemTray(
      iconPath: iconPath,
      title: title,
      toolTip: toolTip,
      isTemplate: isTemplate,
    );
  }

  /// Set system info info
  Future<bool> setSystemTrayInfo({
    String? title,
    String? iconPath,
    String? toolTip,
    bool isTemplate = false,
  }) async {
    return await SystemTrayPlatform.instance.setSystemTrayInfo(
      title: title,
      iconPath: iconPath,
      toolTip: toolTip,
      isTemplate: isTemplate,
    );
  }

  /// (Windows\macOS\Linux) Sets the image associated with this tray icon
  Future<void> setImage(String image, {bool isTemplate = false}) async {
    await setSystemTrayInfo(iconPath: image, isTemplate: isTemplate);
  }

  /// (Windows\macOS) Sets the hover text for this tray icon.
  Future<void> setToolTip(String toolTip) async {
    await setSystemTrayInfo(toolTip: toolTip);
  }

  /// (macOS) Sets the title displayed next to the tray icon in the status bar.
  Future<void> setTitle(String title) async {
    await setSystemTrayInfo(title: title);
  }

  /// (macOS) Returns string - the title displayed next to the tray icon in the status bar
  Future<String> getTitle() async {
    return await SystemTrayPlatform.instance.getTitle();
  }

  /// Sets the native application menu to [menus].
  ///
  /// How exactly this is handled is subject to platform interpretation.
  /// For instance, special menus that are handled entirely on the native
  /// side might be added to the provided menus.
  Future<void> setContextMenu(Menu menu) async {
    await SystemTrayPlatform.instance.setContextMenu(menu.menuId);
  }

  /// Pop up the context menu.
  ///
  Future<void> popUpContextMenu() async {
    await SystemTrayPlatform.instance.popUpContextMenu();
  }

  /// register listener for system tray event.
  void registerSystemTrayEventHandler(SystemTrayEventCallback callback) {
    _systemTrayEventCallback = callback;
  }

  void _callbackHandler(String eventName) {
    if (_systemTrayEventCallback != null) {
      _systemTrayEventCallback!(eventName);
    }
  }

  Future<void> destroy() async {
    await SystemTrayPlatform.instance.destroySystemTray();
  }
}
