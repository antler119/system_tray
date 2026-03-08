import 'dart:async';

import 'package:dart_libayatana_appindicator/dart_libayatana_appindicator.dart';
import 'package:dbus/dbus.dart';
import 'package:uuid/uuid.dart';

import '../menu_item.dart';
import '../utils.dart';
import 'system_tray_platform_interface.dart';

/// An implementation of [SystemTrayPlatform] that uses dart_libayatana_appindicator.
class LinuxSystemTray extends SystemTrayPlatform {
  AppIndicator? _appIndicator;
  final Map<int, List<DBusMenuItem>> _menus = {};
  final Map<int, List<MenuItemBase>> _menuSources = {};
  int? _activeMenuId;

  @override
  Future<bool> initSystemTray({
    required String iconPath,
    String? title,
    String? toolTip,
    bool isTemplate = false,
  }) async {
    final String id = const Uuid().v1();

    String resolvedIcon = await Utils.getIcon(iconPath) ?? iconPath;

    _appIndicator = AppIndicator(
      id: id,
      iconName: resolvedIcon,
      category: AppIndicatorCategory.applicationStatus,
    );

    if (title != null) {
      _appIndicator!.title = title;
    }

    _appIndicator!.status = AppIndicatorStatus.active;

    _appIndicator!.activateEvents.listen((_) {
      onSystemTrayEvent?.call('leftMouseUp');
    });

    return true;
  }

  @override
  Future<bool> setSystemTrayInfo({
    String? title,
    String? iconPath,
    String? toolTip,
    bool isTemplate = false,
  }) async {
    if (_appIndicator == null) return false;

    if (iconPath != null) {
      String resolvedIcon = await Utils.getIcon(iconPath) ?? iconPath;
      _appIndicator!.iconName = resolvedIcon;
    }

    if (title != null) {
      _appIndicator!.title = title;
    }

    return true;
  }

  @override
  Future<String> getTitle() async {
    return _appIndicator?.title ?? "";
  }

  @override
  Future<void> destroySystemTray() async {
    if (_appIndicator != null) {
      _appIndicator!.status = AppIndicatorStatus.passive;
    }
    _appIndicator = null;
    _menus.clear();
    _menuSources.clear();
  }

  @override
  Future<bool> createContextMenu(int menuId, List<MenuItemBase> menus) async {
    _menuSources[menuId] = menus;
    await _updateMenu(menuId);
    return true;
  }

  Future<void> _updateMenu(int menuId) async {
    if (_appIndicator == null) return;

    final source = _menuSources[menuId];
    if (source == null) return;

    final items = <DBusMenuItem>[];
    for (final item in source) {
      items.add(await _buildDBusItem(item));
    }

    _menus[menuId] = items;

    if (_activeMenuId == menuId) {
      _appIndicator!.setMenu(items);
    }
  }

  Future<DBusMenuItem> _buildDBusItem(MenuItemBase item) async {
    final properties = <String, DBusValue>{};
    final children = <DBusMenuItem>[];

    properties['enabled'] = DBusBoolean(item.enabled);
    properties['label'] = DBusString(item.label);

    if (item.image != null) {
      String? iconPath = await Utils.getIcon(item.image);
      if (iconPath != null) {
        properties['icon-name'] = DBusString(iconPath);
      }
    }

    if (item is MenuItemCheckbox) {
      properties['toggle-type'] = const DBusString('checkmark');
      properties['toggle-state'] = DBusInt32(item.checked ? 1 : 0);
    } else if (item is SubMenu) {
      properties['children-display'] = const DBusString('submenu');
      for (final child in item.children) {
        children.add(await _buildDBusItem(child));
      }
    } else if (item is MenuSeparator) {
      properties['type'] = const DBusString('separator');
    }

    void Function()? onActivated;
    if (item is! MenuSeparator && item is! SubMenu) {
      onActivated = () {
        if (item.menuId != null && item.menuItemId != null) {
          onMenuItemSelected?.call(item.menuId!, item.menuItemId!);
        }
      };
    }

    int id = item.menuItemId ?? 0;

    return DBusMenuItem(
      id: id,
      properties: properties,
      children: children,
      onActivated: onActivated,
    );
  }

  @override
  Future<void> setContextMenu(int menuId) async {
    _activeMenuId = menuId;
    if (_appIndicator != null && _menus.containsKey(menuId)) {
      _appIndicator!.setMenu(_menus[menuId]!);
    } else if (_menuSources.containsKey(menuId)) {
      await _updateMenu(menuId);
    }
  }

  @override
  Future<void> popUpContextMenu() async {
    // Not supported
  }

  MenuItemBase? _findItem(int menuId, int menuItemId) {
    final source = _menuSources[menuId];
    if (source == null) return null;
    return _findItemInList(source, menuItemId);
  }

  MenuItemBase? _findItemInList(List<MenuItemBase> items, int id) {
    for (final item in items) {
      if (item.menuItemId == id) return item;
      if (item is SubMenu) {
        final found = _findItemInList(item.children, id);
        if (found != null) return found;
      }
    }
    return null;
  }

  @override
  Future<void> setMenuItemLabel(int menuId, int menuItemId, String label) async {
    final item = _findItem(menuId, menuItemId);
    if (item != null) {
      item.label = label;
      await _updateMenu(menuId);
    }
  }

  @override
  Future<void> setMenuItemImage(int menuId, int menuItemId, String image) async {
    final item = _findItem(menuId, menuItemId);
    if (item != null) {
      item.image = image;
      await _updateMenu(menuId);
    }
  }

  @override
  Future<void> setMenuItemEnable(int menuId, int menuItemId, bool enabled) async {
    final item = _findItem(menuId, menuItemId);
    if (item != null) {
      item.enabled = enabled;
      await _updateMenu(menuId);
    }
  }

  @override
  Future<void> setMenuItemCheck(int menuId, int menuItemId, bool checked) async {
    final item = _findItem(menuId, menuItemId);
    if (item != null) {
      item.checked = checked;
      await _updateMenu(menuId);
    }
  }
}
