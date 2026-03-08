import 'dart:async';

import 'package:dart_xdg_status_notifier_item/dart_xdg_status_notifier_item.dart';

import 'constants.dart';
import 'menu.dart';
import 'menu_item.dart';

class SystemTrayLinux {
  static final Map<int, DBusMenuItem> _menuMap = {};
  static StatusNotifierItemClient? _client;

  static void Function(String eventName)? systemTrayEventCallback;
  static void Function(int menuId, int menuItemId)? menuItemSelectedCallback;

  static Future<bool> initSystemTray({
    required String trayId,
    required String iconPath,
    String? title,
    String? toolTip,
    bool isTemplate = false,
  }) async {
    _client = StatusNotifierItemClient(
        id: trayId,
        iconName: iconPath,
        title: title ?? '',
        menu: DBusMenuItem(children: []),
        onContextMenu: (x, y) async {
          if (systemTrayEventCallback != null) {
            systemTrayEventCallback!(kSystemTrayEventRightClick);
          }
        },
        onActivate: (x, y) async {
          if (systemTrayEventCallback != null) {
            systemTrayEventCallback!(kSystemTrayEventClick);
          }
        },
        onSecondaryActivate: (x, y) async {
          if (systemTrayEventCallback != null) {
            systemTrayEventCallback!(kSystemTrayEventDoubleClick);
          }
        },
        onScroll: (delta, orientation) async {
          if (systemTrayEventCallback != null) {
            systemTrayEventCallback!(kSystemTrayEventScroll);
          }
        });

    if (toolTip != null && toolTip.isNotEmpty) {
      _client!.toolTip = StatusNotifierToolTip(
          iconName: 'icon-name', title: toolTip, body: '', iconPixmap: []);
    }

    await _client!.connect();
    return true;
  }

  static Future<bool> setSystemTrayInfo({
    String? title,
    String? iconPath,
    String? toolTip,
    bool isTemplate = false,
  }) async {
    if (_client != null) {
      if (iconPath != null) _client!.iconName = iconPath;
      if (title != null) _client!.title = title;
      if (toolTip != null) {
        _client!.toolTip = StatusNotifierToolTip(
            iconName: 'icon-name', title: toolTip, body: '', iconPixmap: []);
      }
    }
    return true;
  }

  static Future<void> setContextMenu(int menuId) async {
    if (_menuMap.containsKey(menuId) && _client != null) {
      await _client!.updateMenu(_menuMap[menuId]!);
    }
  }

  static Future<void> popUpContextMenu() async {
    // Not typically supported directly via xdg_status_notifier_item without shell interaction
  }

  static Future<String> getTitle() async {
    return _client?.title ?? "";
  }

  static Future<void> destroySystemTray() async {
    if (_client != null) {
      await _client!.close();
      _client = null;
    }
  }

  static DBusMenuItem _buildMenuItem(MenuItemBase item, int menuId) {
    if (item is MenuSeparator) {
      return DBusMenuItem.separator();
    }

    if (item is SubMenu) {
      final children =
          item.children.map((e) => _buildMenuItem(e, menuId)).toList();
      return DBusMenuItem(
        label: item.label,
        enabled: item.enabled,
        children: children,
      );
    }

    if (item is MenuItemCheckbox) {
      return DBusMenuItem.checkmark(item.label,
          state: item.checked, enabled: item.enabled, onClicked: () async {
        if (menuItemSelectedCallback != null) {
          menuItemSelectedCallback!(menuId, item.menuItemId ?? -1);
        }
      });
    }

    return DBusMenuItem(
        label: item.label,
        enabled: item.enabled,
        onClicked: () async {
          if (menuItemSelectedCallback != null) {
            menuItemSelectedCallback!(menuId, item.menuItemId ?? -1);
          }
        });
  }

  static Future<bool> buildMenu(int menuId, List<MenuItemBase> menus) async {
    final children = menus.map((e) => _buildMenuItem(e, menuId)).toList();
    final menu = DBusMenuItem(children: children);
    _menuMap[menuId] = menu;

    return true;
  }

  static Future<void> setMenuItemLabel(
      int menuId, int menuItemId, String label) async {}

  static Future<void> setMenuItemImage(
      int menuId, int menuItemId, String imageAbsolutePath) async {}

  static Future<void> setMenuItemEnable(
      int menuId, int menuItemId, bool enabled) async {}

  static Future<void> setMenuItemCheck(
      int menuId, int menuItemId, bool checked) async {}
}
