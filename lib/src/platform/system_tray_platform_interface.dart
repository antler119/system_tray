import 'dart:async';
import 'dart:io';

import '../menu_item.dart';
import 'system_tray_method_channel.dart';
import 'system_tray_linux.dart';

/// The interface that implementations of system_tray must implement.
abstract class SystemTrayPlatform {
  static SystemTrayPlatform? _instance;

  /// The default instance of [SystemTrayPlatform] to use.
  ///
  /// Defaults to [MethodChannelSystemTray] on non-Linux, and [LinuxSystemTray] on Linux.
  static SystemTrayPlatform get instance {
    if (_instance == null) {
      if (Platform.isLinux) {
        _instance = LinuxSystemTray();
      } else {
        _instance = MethodChannelSystemTray();
      }
    }
    return _instance!;
  }

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [SystemTrayPlatform] when
  /// they register themselves.
  static set instance(SystemTrayPlatform instance) {
    _instance = instance;
  }

  /// Initializes the system tray.
  Future<bool> initSystemTray({
    required String iconPath,
    String? title,
    String? toolTip,
    bool isTemplate = false,
  }) {
    throw UnimplementedError('initSystemTray() has not been implemented.');
  }

  /// Sets system tray info.
  Future<bool> setSystemTrayInfo({
    String? title,
    String? iconPath,
    String? toolTip,
    bool isTemplate = false,
  }) {
    throw UnimplementedError('setSystemTrayInfo() has not been implemented.');
  }

  /// Returns the title displayed next to the tray icon in the status bar.
  Future<String> getTitle() {
    throw UnimplementedError('getTitle() has not been implemented.');
  }

  /// Sets the context menu.
  Future<void> setContextMenu(int menuId) {
    throw UnimplementedError('setContextMenu() has not been implemented.');
  }

  /// Pops up the context menu.
  Future<void> popUpContextMenu() {
    throw UnimplementedError('popUpContextMenu() has not been implemented.');
  }

  /// Destroys the system tray.
  Future<void> destroySystemTray() {
    throw UnimplementedError('destroySystemTray() has not been implemented.');
  }

  /// Creates a context menu.
  Future<bool> createContextMenu(int menuId, List<MenuItemBase> menus) {
    throw UnimplementedError('createContextMenu() has not been implemented.');
  }

  /// Sets label for a menu item.
  Future<void> setMenuItemLabel(int menuId, int menuItemId, String label) {
    throw UnimplementedError('setMenuItemLabel() has not been implemented.');
  }

  /// Sets image for a menu item.
  Future<void> setMenuItemImage(int menuId, int menuItemId, String image) {
    throw UnimplementedError('setMenuItemImage() has not been implemented.');
  }

  /// Sets enable state for a menu item.
  Future<void> setMenuItemEnable(int menuId, int menuItemId, bool enabled) {
    throw UnimplementedError('setMenuItemEnable() has not been implemented.');
  }

  /// Sets check state for a menu item.
  Future<void> setMenuItemCheck(int menuId, int menuItemId, bool checked) {
    throw UnimplementedError('setMenuItemCheck() has not been implemented.');
  }

  /// Callback for system tray events.
  void Function(String eventName)? onSystemTrayEvent;

  /// Callback for menu item selection.
  void Function(int menuId, int menuItemId)? onMenuItemSelected;
}
