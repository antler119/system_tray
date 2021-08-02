import 'dart:async';

import 'package:flutter/services.dart';

const String _kChannelName = "flutter/system_tray/app_window";

const String _kInitAppWindow = "InitAppWindow";
const String _kShowAppWindow = "ShowAppWindow";
const String _kHideAppWindow = "HideAppWindow";
const String _kCloseAppWindow = "CloseAppWindow";

class AppWindow {
  AppWindow() {
    _platformChannel.setMethodCallHandler(_callbackHandler);
    _init();
  }

  static const MethodChannel _platformChannel = MethodChannel(_kChannelName);

  Future<void> show() async {
    await _platformChannel.invokeMethod(_kShowAppWindow);
  }

  Future<void> hide() async {
    await _platformChannel.invokeMethod(_kHideAppWindow);
  }

  Future<void> close() async {
    await _platformChannel.invokeMethod(_kCloseAppWindow);
  }

  void _init() async {
    await _platformChannel.invokeMethod(_kInitAppWindow);
  }

  Future<void> _callbackHandler(MethodCall methodCall) async {}
}
