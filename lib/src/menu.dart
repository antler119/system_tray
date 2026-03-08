import 'package:flutter/material.dart';

import 'menu_item.dart';
import 'platform/system_tray_platform_interface.dart';

class Menu {
  static final Map<int, Menu> _menuMap = {};

  /// The ID to use the next time a menu needs an ID assigned.
  static int _nextMenuId = 1;

  List<MenuItemBase>? _menus;

  int _menuId = 1;

  int _menuItemId = 1;

  bool _updateInProgress = false;

  Menu() {
    SystemTrayPlatform.instance.onMenuItemSelected ??= _callbackHandler;
  }

  int get menuId => _menuId;

  int get nextMenuItemId {
    return _menuItemId++;
  }

  Future<bool> buildFrom(List<MenuItemBase> menus) async {
    _menuId = _nextMenuId++;
    _menus = menus;
    _menuMap.putIfAbsent(_menuId, () => this);

    _prepareMenuItems(menus);

    _updateInProgress = true;
    bool result = await SystemTrayPlatform.instance.createContextMenu(_menuId, menus);
    _updateInProgress = false;

    return result;
  }

  void _prepareMenuItems(List<MenuItemBase> menus) {
    _menuItemId = 1;
    _assignIds(menus);
  }

  void _assignIds(List<MenuItemBase> menus) {
    for (final menuItem in menus) {
      menuItem.menuId = menuId;
      menuItem.menuItemId = nextMenuItemId;

      if (menuItem is SubMenu) {
        _assignIds(menuItem.children);
      }
    }
  }

  T? findItemByName<T>(final String name) {
    return _findItemByName(name, _menus!) as T;
  }

  MenuItemBase? _findItemByName(
      final String name, final List<MenuItemBase> menus) {
    MenuItemBase? item;
    for (final menuItem in menus) {
      if (menuItem is SubMenu) {
        item = _findItemByName(name, menuItem.children);
      } else if (menuItem.name == name) {
        item = menuItem;
      }

      if (item != null) {
        break;
      }
    }
    return item;
  }

  MenuItemBase? _findItemById(
      final int? menuItemId, final List<MenuItemBase>? menus) {
    MenuItemBase? item;
    if (menuItemId != null && menus != null) {
      for (final menuItem in menus) {
        if (menuItem is SubMenu) {
          item = _findItemById(menuItemId, menuItem.children);
        } else if (menuItem.menuItemId == menuItemId) {
          item = menuItem;
        }

        if (item != null) {
          break;
        }
      }
    }
    return item;
  }

  static void _callbackHandler(int menuId, int menuItemId) {
    final Menu? menu = _menuMap[menuId];
    if (menu != null) {
      if (menu._updateInProgress) {
        debugPrint(
            'Warning: Menu selection callback received during menu update.');
        return;
      }

      final MenuItemBase? menuItem =
          menu._findItemById(menuItemId, menu._menus);

      debugPrint('MenuItemBase select menuId:$menuId menuItemId:$menuItemId');

      final callback = menuItem?.onClicked;
      if (callback != null) {
        callback(menuItem!);
      }
    }
  }
}
