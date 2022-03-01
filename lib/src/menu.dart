import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'menu_item.dart';
import 'utils.dart';

const String _kChannelName = "flutter/system_tray/menu_manager";

const String _kCreateContextMenu = "createContextMenu";

const String _kMenuItemSelectedCallbackMethod = 'MenuItemSelectedCallback';

const String _kMenuIdKey = 'menu_id';
const String _kMenuItemIdKey = 'menu_item_id';
const String _kMenuListKey = 'menu_list';
const String _kIdKey = 'id';
const String _kTypeKey = 'type';
const String _kLabelKey = 'label';
const String _kImageKey = 'image';
const String _kSubMenuKey = 'submenu';
const String _kEnabledKey = 'enabled';
const String _kCheckedKey = 'checked';

class Menu {
  static const MethodChannel _platformChannel = MethodChannel(_kChannelName);

  /// Map from unique identifiers assigned by this class to the callbacks for
  /// those menu items.
  static final Map<int, Map<int, MenuItemSelectedCallback>>
      _selectionCallbacks = {};

  /// The ID to use the next time a menu needs an ID assigned.
  static int nextMenuId = 1;

  List<MenuItem>? _menus;

  int _menuId = -1;

  /// The ID to use the next time a menu item needs an ID assigned.
  int _nextMenuItemId = 1;

  /// Whether or not a call to [_kMenuSetMethod] is outstanding.
  ///
  /// This is used to drop any menu callbacks that aren't received until
  /// after a new call to setMenu, so that clients don't received unexpected
  /// stale callbacks.
  bool _updateInProgress = false;

  Menu() {
    _platformChannel.setMethodCallHandler(_callbackHandler);
    _menuId = nextMenuId++;
  }

  int get menuId => _menuId;

  Future<bool> buildFrom(List<MenuItem> menus) async {
    _menus = menus;
    return await _createContextMenu(_menus!);
  }

  MenuItem? findItemByName(String name) {
    return _findItemByName(name, _menus!);
  }

  MenuItem? _findItemByName(String name, List<MenuItem> menus) {
    MenuItem? item;
    for (final menuItem in menus) {
      if (menuItem is SubMenu) {
        item = _findItemByName(name, menuItem.children);
      } else if (menuItem is! MenuSeparator) {
        if (menuItem.name == name) {
          item = menuItem;
        }
      }

      if (item != null) {
        break;
      }
    }
    return item;
  }

  Future<bool> _createContextMenu(List<MenuItem> menus) async {
    bool result = false;
    try {
      _updateInProgress = true;
      result = await _platformChannel
          .invokeMethod(_kCreateContextMenu, <String, dynamic>{
        _kMenuIdKey: _menuId,
        _kMenuListKey: _channelRepresentationForMenus(menus),
      });
      _updateInProgress = false;
    } on PlatformException catch (e) {
      debugPrint('Platform exception create context menu: ${e.message}');
    }
    return result;
  }

  /// Converts [menus] to a representation that can be sent in the arguments to
  /// [_kMenuSetMethod].
  ///
  /// As a side-effect, repopulates _selectionCallbacks with a mapping from
  /// the IDs assigned to any menu item with a selection handler to the
  /// callback that should be triggered.
  List<dynamic> _channelRepresentationForMenus(List<MenuItem> menus) {
    _selectionCallbacks[_menuId]?.clear();
    _nextMenuItemId = 1;

    return menus.map(_channelRepresentationForMenuItem).toList();
  }

  /// Returns a representation of [item] suitable for passing over the
  /// platform channel to the native plugin.
  Map<String, dynamic> _channelRepresentationForMenuItem(MenuItem item) {
    final representation = <String, dynamic>{};

    item.channel = _platformChannel;
    item.menuId = _menuId;

    if (item is MenuSeparator) {
      representation[_kTypeKey] = item.type;
    } else {
      representation[_kLabelKey] = item.label;
      String? icon = Utils.getIcon(item.image);
      representation[_kImageKey] = icon;

      if (item is SubMenu) {
        representation[_kTypeKey] = item.type;
        representation[_kSubMenuKey] =
            _channelRepresentationForMenu(item.children);
      } else if (item is MenuItemLable) {
        representation[_kTypeKey] = item.type;
        final handler = item.onClicked;
        if (handler != null) {
          item.menuItemId = _storeMenuCallback(handler);
          representation[_kIdKey] = item.menuItemId;
        }
        representation[_kEnabledKey] = item.enabled;
      } else if (item is MenuItemCheckbox) {
        representation[_kTypeKey] = item.type;
        final handler = item.onClicked;
        if (handler != null) {
          item.menuItemId = _storeMenuCallback(handler);
          representation[_kIdKey] = item.menuItemId;
        }
        representation[_kEnabledKey] = item.enabled;
        representation[_kCheckedKey] = item.checked;
      } else {
        throw ArgumentError(
            'Unknown MenuItemBase type: $item (${item.runtimeType})');
      }
    }
    return representation;
  }

  /// Returns the representation of [menu] suitable for passing over the
  /// platform channel to the native plugin.
  List<dynamic> _channelRepresentationForMenu(List<MenuItem> menus) {
    final menuItemRepresentations = [];
    // Dividers are only allowed after non-divider items (see ApplicationMenu).
    var skipNextDivider = true;
    for (final menuItem in menus) {
      final isDivider = menuItem is MenuSeparator;
      if (isDivider && skipNextDivider) {
        continue;
      }
      skipNextDivider = isDivider;
      menuItemRepresentations.add(_channelRepresentationForMenuItem(menuItem));
    }
    // If the last item is a divider, remove it (see ApplicationMenu).
    if (skipNextDivider && menuItemRepresentations.isNotEmpty) {
      menuItemRepresentations.removeLast();
    }
    return menuItemRepresentations;
  }

  /// Stores [callback] for use plugin callback handling, returning the ID
  /// under which it was stored.
  ///
  /// The returned ID should be attached to the menu so that the native plugin
  /// can identify the menu item selected in the callback.
  int _storeMenuCallback(MenuItemSelectedCallback callback) {
    final id = _nextMenuItemId++;
    _selectionCallbacks
        .putIfAbsent(_menuId, () => (<int, MenuItemSelectedCallback>{}))
        .update(id, (value) => callback, ifAbsent: () => callback);
    return id;
  }

  Future<void> _callbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kMenuItemSelectedCallbackMethod) {
      if (_updateInProgress) {
        // Drop stale callbacks.
        // TODO: Evaluate whether this works in practice, or if races are
        // regular occurences that clients will need to be prepared to
        // handle (in which case a more complex ID system will be needed).
        debugPrint(
            'Warning: Menu selection callback received during menu update.');
        return;
      }
      final int? menuId = methodCall.arguments[_kMenuIdKey];
      final int? menuItemId = methodCall.arguments[_kMenuItemIdKey];
      debugPrint('MenuItem select menuId:$menuId menuItemId:$menuItemId');
      final callback = _selectionCallbacks[menuId]?[menuItemId];
      if (callback == null) {
        throw Exception('Unknown menu item ID $menuId:$menuItemId');
      }
      callback();
    }
  }
}
