import 'dart:async';

import 'package:flutter/services.dart';

import 'menu.dart';
import 'utils.dart';

const String _kChannelName = "flutter/system_tray/tray";

const String _kInitSystemTray = "InitSystemTray";
const String _kSetSystemTrayInfo = "SetSystemTrayInfo";
const String _kSetContextMenu = "SetContextMenu";
const String _kPopupContextMenu = "PopupContextMenu";
const String _kGetTitle = "GetTitle";

const String _kSystemTrayEventCallbackMethod = 'SystemTrayEventCallback';

const String _kTitleKey = "title";
const String _kIconPathKey = "iconpath";
const String _kToolTipKey = "tooltip";

/// A callback provided to [SystemTray] to handle system tray click event.
typedef SystemTrayEventCallback = void Function(String eventName);

/// Representation of system tray
class SystemTray {
  SystemTray() {
    _platformChannel.setMethodCallHandler(_callbackHandler);
  }

  static const MethodChannel _platformChannel = MethodChannel(_kChannelName);

  ///
  SystemTrayEventCallback? _systemTrayEventCallback;

  /// Show a SystemTray icon
  Future<bool> initSystemTray({
    required String iconPath,
    String? title,
    String? toolTip,
  }) async {
    bool value = await _platformChannel.invokeMethod(
      _kInitSystemTray,
      <String, dynamic>{
        _kTitleKey: title,
        _kIconPathKey: await Utils.getIcon2(iconPath),
        _kToolTipKey: toolTip,
      },
    );
    return value;
  }

  /// Set system info info
  Future<bool> setSystemTrayInfo({
    String? title,
    String? iconPath,
    String? toolTip,
  }) async {
    bool value = await _platformChannel.invokeMethod(
      _kSetSystemTrayInfo,
      <String, dynamic>{
        _kTitleKey: title,
        _kIconPathKey: await Utils.getIcon2(iconPath),
        _kToolTipKey: toolTip,
      },
    );
    return value;
  }

  /// (Windows\macOS\Linux) Sets the image associated with this tray icon
  Future<void> setImage(String image) async {
    await setSystemTrayInfo(iconPath: image);
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
    return await _platformChannel.invokeMethod(_kGetTitle);
  }

  /// register listener for system tray event.
  void registerSystemTrayEventHandler(SystemTrayEventCallback callback) {
    _systemTrayEventCallback = callback;
  }

  /// Sets the native application menu to [menus].
  ///
  /// How exactly this is handled is subject to platform interpretation.
  /// For instance, special menus that are handled entirely on the native
  /// side might be added to the provided menus.
  Future<void> setContextMenu(Menu menu) async {
    await _platformChannel.invokeMethod(_kSetContextMenu, menu.menuId);
  }

  /// Pop up the context menu.
  ///
  Future<void> popUpContextMenu() async {
    await _platformChannel.invokeMethod(_kPopupContextMenu);
  }

  Future<void> _callbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kSystemTrayEventCallbackMethod) {
      if (_systemTrayEventCallback != null) {
        final String eventName = methodCall.arguments;
        _systemTrayEventCallback!(eventName);
      }
    }
  }
}
