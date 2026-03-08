import 'dart:async';

import 'package:flutter/services.dart';
import 'package:uuid/uuid.dart';

import '../menu_item.dart';
import '../utils.dart';
import 'system_tray_platform_interface.dart';

const String _kTrayChannelName = "flutter/system_tray/tray";
const String _kMenuChannelName = "flutter/system_tray/menu_manager";

// Tray methods
const String _kInitSystemTray = "InitSystemTray";
const String _kSetSystemTrayInfo = "SetSystemTrayInfo";
const String _kSetContextMenu = "SetContextMenu";
const String _kPopupContextMenu = "PopupContextMenu";
const String _kGetTitle = "GetTitle";
const String _kDestroySystemTray = "DestroySystemTray";
const String _kSystemTrayEventCallbackMethod = 'SystemTrayEventCallback';

// Menu methods
const String _kCreateContextMenu = "CreateContextMenu";
const String _kMenuItemSelectedCallbackMethod = 'MenuItemSelectedCallback';
const String _kSetLabel = "SetLabel";
const String _kSetImage = "SetImage";
const String _kSetEnable = "SetEnable";
const String _kSetCheck = "SetCheck";

// Keys
const String _kTrayIdKey = "tray_id";
const String _kTitleKey = "title";
const String _kIconPathKey = "iconpath";
const String _kToolTipKey = "tooltip";
const String _kIsTemplateKey = "is_template";
const String _kMenuIdKey = 'menu_id';
const String _kMenuItemIdKey = 'menu_item_id';
const String _kMenuListKey = 'menu_list';
const String _kLabelKey = 'label';
const String _kImageKey = 'image';
const String _kEnabledKey = 'enabled';
const String _kCheckedKey = 'checked';

/// An implementation of [SystemTrayPlatform] that uses method channels.
class MethodChannelSystemTray extends SystemTrayPlatform {
  /// The method channel used to interact with the system tray.
  final MethodChannel _trayChannel = const MethodChannel(_kTrayChannelName);

  /// The method channel used to interact with the menu manager.
  final MethodChannel _menuChannel = const MethodChannel(_kMenuChannelName);

  MethodChannelSystemTray() {
    _trayChannel.setMethodCallHandler(_trayCallbackHandler);
    _menuChannel.setMethodCallHandler(_menuCallbackHandler);
  }

  Future<void> _trayCallbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kSystemTrayEventCallbackMethod) {
      final String eventName = methodCall.arguments;
      onSystemTrayEvent?.call(eventName);
    }
  }

  Future<void> _menuCallbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kMenuItemSelectedCallbackMethod) {
      final int menuId = methodCall.arguments[_kMenuIdKey];
      final int menuItemId = methodCall.arguments[_kMenuItemIdKey];
      onMenuItemSelected?.call(menuId, menuItemId);
    }
  }

  @override
  Future<bool> initSystemTray({
    required String iconPath,
    String? title,
    String? toolTip,
    bool isTemplate = false,
  }) async {
    return await _trayChannel.invokeMethod(
      _kInitSystemTray,
      <String, dynamic>{
        _kTrayIdKey: const Uuid().v1(),
        _kTitleKey: title,
        _kIconPathKey: await Utils.getIcon(iconPath),
        _kToolTipKey: toolTip,
        _kIsTemplateKey: isTemplate,
      },
    );
  }

  @override
  Future<bool> setSystemTrayInfo({
    String? title,
    String? iconPath,
    String? toolTip,
    bool isTemplate = false,
  }) async {
    return await _trayChannel.invokeMethod(
      _kSetSystemTrayInfo,
      <String, dynamic>{
        _kTitleKey: title,
        _kIconPathKey: await Utils.getIcon(iconPath),
        _kToolTipKey: toolTip,
        _kIsTemplateKey: isTemplate,
      },
    );
  }

  @override
  Future<String> getTitle() async {
    return await _trayChannel.invokeMethod(_kGetTitle);
  }

  @override
  Future<void> setContextMenu(int menuId) async {
    await _trayChannel.invokeMethod(_kSetContextMenu, menuId);
  }

  @override
  Future<void> popUpContextMenu() async {
    await _trayChannel.invokeMethod(_kPopupContextMenu);
  }

  @override
  Future<void> destroySystemTray() async {
    await _trayChannel.invokeMethod(_kDestroySystemTray);
  }

  @override
  Future<bool> createContextMenu(int menuId, List<MenuItemBase> menus) async {
    await _resolveMenuImages(menus);
    return await _menuChannel.invokeMethod(_kCreateContextMenu, <String, dynamic>{
      _kMenuIdKey: menuId,
      _kMenuListKey: menus.map((e) => e.toJson()).toList(),
    });
  }

  Future<void> _resolveMenuImages(List<MenuItemBase> menus) async {
    for (final menuItem in menus) {
      menuItem.imageAbsolutePath = await Utils.getIcon(menuItem.image);
      if (menuItem is SubMenu) {
        await _resolveMenuImages(menuItem.children);
      }
    }
  }

  @override
  Future<void> setMenuItemLabel(int menuId, int menuItemId, String label) async {
    await _menuChannel.invokeMethod(_kSetLabel, {
      _kMenuIdKey: menuId,
      _kMenuItemIdKey: menuItemId,
      _kLabelKey: label,
    });
  }

  @override
  Future<void> setMenuItemImage(int menuId, int menuItemId, String image) async {
    String? imageAbsolutePath = await Utils.getIcon(image);
    await _menuChannel.invokeMethod(_kSetImage, {
      _kMenuIdKey: menuId,
      _kMenuItemIdKey: menuItemId,
      _kImageKey: imageAbsolutePath,
    });
  }

  @override
  Future<void> setMenuItemEnable(int menuId, int menuItemId, bool enabled) async {
    await _menuChannel.invokeMethod(_kSetEnable, {
      _kMenuIdKey: menuId,
      _kMenuItemIdKey: menuItemId,
      _kEnabledKey: enabled,
    });
  }

  @override
  Future<void> setMenuItemCheck(int menuId, int menuItemId, bool checked) async {
    await _menuChannel.invokeMethod(_kSetCheck, {
      _kMenuIdKey: menuId,
      _kMenuItemIdKey: menuItemId,
      _kCheckedKey: checked,
    });
  }
}
