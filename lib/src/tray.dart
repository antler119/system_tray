import 'dart:async';

import 'package:flutter/services.dart';

import 'menu_item.dart';

const String _kChannelName = "flutter/system_tray";

const String _kInitSystemTray = "InitSystemTray";
const String _kSetContextMenu = "SetContextMenu";
const String _kMenuItemSelectedCallbackMethod = 'MenuItemSelectedCallback';

const String _kIdKey = 'id';
const String _kTypeKey = 'type';
const String _kLabelKey = 'label';
const String _kSubMenuKey = 'submenu';
const String _kEnabledKey = 'enabled';

class SystemTray {
  SystemTray() {
    _platformChannel.setMethodCallHandler(_callbackHandler);
  }

  static const MethodChannel _platformChannel = MethodChannel(_kChannelName);

  /// Map from unique identifiers assigned by this class to the callbacks for
  /// those menu items.
  final Map<int, MenuItemSelectedCallback> _selectionCallbacks = {};

  /// The ID to use the next time a menu item needs an ID assigned.
  int _nextMenuItemId = 1;

  /// Whether or not a call to [_kMenuSetMethod] is outstanding.
  ///
  /// This is used to drop any menu callbacks that aren't received until
  /// after a new call to setMenu, so that clients don't received unexpected
  /// stale callbacks.
  bool _updateInProgress = false;

  // Show a SystemTray icon
  Future<bool> initSystemTray(String title, {String? iconPath}) async {
    bool value = await _platformChannel.invokeMethod(
      _kInitSystemTray,
      <String, dynamic>{
        "title": title,
        "iconpath": iconPath ?? "",
      },
    );
    return value;
  }

  /// Sets the native application menu to [menus].
  ///
  /// How exactly this is handled is subject to platform interpretation.
  /// For instance, special menus that are handled entirely on the native
  /// side might be added to the provided menus.
  Future<void> setContextMenu(List<MenuItemBase> menus) async {
    try {
      _updateInProgress = true;
      await _platformChannel.invokeMethod(
          _kSetContextMenu, _channelRepresentationForMenus(menus));
      _updateInProgress = false;
    } on PlatformException catch (e) {
      print('Platform exception setting menu: ${e.message}');
    }
  }

  /// Converts [menus] to a representation that can be sent in the arguments to
  /// [_kMenuSetMethod].
  ///
  /// As a side-effect, repopulates _selectionCallbacks with a mapping from
  /// the IDs assigned to any menu item with a selection handler to the
  /// callback that should be triggered.
  List<dynamic> _channelRepresentationForMenus(List<MenuItemBase> menus) {
    _selectionCallbacks.clear();
    _nextMenuItemId = 1;

    return menus.map(_channelRepresentationForMenuItem).toList();
  }

  /// Returns a representation of [item] suitable for passing over the
  /// platform channel to the native plugin.
  Map<String, dynamic> _channelRepresentationForMenuItem(MenuItemBase item) {
    final representation = <String, dynamic>{};
    if (item is MenuSeparator) {
      representation[_kTypeKey] = item.type;
    } else {
      representation[_kLabelKey] = item.label;
      if (item is SubMenu) {
        representation[_kTypeKey] = item.type;
        representation[_kSubMenuKey] =
            _channelRepresentationForMenu(item.children);
      } else if (item is MenuItem) {
        representation[_kTypeKey] = item.type;
        final handler = item.onClicked;
        if (handler != null) {
          representation[_kIdKey] = _storeMenuCallback(handler);
        }
        if (!item.enabled) {
          representation[_kEnabledKey] = false;
        }
      } else {
        throw ArgumentError(
            'Unknown MenuItemBase type: $item (${item.runtimeType})');
      }
    }
    return representation;
  }

  /// Returns the representation of [menu] suitable for passing over the
  /// platform channel to the native plugin.
  List<dynamic> _channelRepresentationForMenu(List<MenuItemBase> menu) {
    final menuItemRepresentations = [];
    // Dividers are only allowed after non-divider items (see ApplicationMenu).
    var skipNextDivider = true;
    for (final menuItem in menu) {
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
    _selectionCallbacks[id] = callback;
    return id;
  }

  Future<void> _callbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kMenuItemSelectedCallbackMethod) {
      if (_updateInProgress) {
        // Drop stale callbacks.
        // TODO: Evaluate whether this works in practice, or if races are
        // regular occurences that clients will need to be prepared to
        // handle (in which case a more complex ID system will be needed).
        print('Warning: Menu selection callback received during menu update.');
        return;
      }
      final int menuItemId = methodCall.arguments;
      final callback = _selectionCallbacks[menuItemId];
      if (callback == null) {
        throw Exception('Unknown menu item ID $menuItemId');
      }
      callback();
    }
  }
}
