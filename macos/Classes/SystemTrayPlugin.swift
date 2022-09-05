import Cocoa
import FlutterMacOS

private let kChannelNameAppWindow = "flutter/system_tray/app_window"
private let kChannelNameMenuManager = "flutter/system_tray/menu_manager"
private let kChannelNameTray = "flutter/system_tray/tray"

public class SystemTrayPlugin: NSObject, FlutterPlugin {
  var registrar: FlutterPluginRegistrar
  var app_window: AppWindow
  var tray: Tray
  var menu_manager: MenuManager

  public static func register(with registrar: FlutterPluginRegistrar) {
    let channelAppWindow = FlutterMethodChannel(
      name: kChannelNameAppWindow, binaryMessenger: registrar.messenger)
    let channelMenuManager = FlutterMethodChannel(
      name: kChannelNameMenuManager, binaryMessenger: registrar.messenger)
    let channelTray = FlutterMethodChannel(
      name: kChannelNameTray, binaryMessenger: registrar.messenger)

    let instance = SystemTrayPlugin(registrar, channelAppWindow, channelMenuManager, channelTray)
    registrar.addMethodCallDelegate(instance, channel: channelAppWindow)
    registrar.addMethodCallDelegate(instance, channel: channelMenuManager)
    registrar.addMethodCallDelegate(instance, channel: channelTray)
  }

  init(
    _ registrar: FlutterPluginRegistrar, _ channelAppWindow: FlutterMethodChannel,
    _ channelMenuManager: FlutterMethodChannel, _ channelTray: FlutterMethodChannel
  ) {
    self.registrar = registrar
    self.app_window = AppWindow(registrar, channelAppWindow)
    self.menu_manager = MenuManager(registrar, channelMenuManager)
    self.tray = Tray(registrar, channelTray, self.menu_manager)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case kInitAppWindow, kShowAppWindow, kHideAppWindow, kCloseAppWindow:
      app_window.handle(call, result: result)
    case kCreateContextMenu, kSetLabel, kSetImage, kSetEnable, kSetCheck:
      menu_manager.handle(call, result: result)
    case kInitSystemTray, kSetSystemTrayInfo, kSetContextMenu, kPopupContextMenu, kSetPressedImage,
      kGetTitle, kDestroySystemTray:
      tray.handle(call, result: result)
    default:
      result(FlutterMethodNotImplemented)
    }
  }
}
