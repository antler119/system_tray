import Cocoa
import FlutterMacOS

let kChannelName = "flutter/system_tray"

let kInitSystemTray = "InitSystemTray"
let kSetSystemTrayInfo = "SetSystemTrayInfo"
let kSetContextMenu = "SetContextMenu"
let kPopupContextMenu = "PopupContextMenu"

let kMenuItemSelectedCallbackMethod = "MenuItemSelectedCallback"

let kTitleKey = "title"
let kBase64IconKey = "base64icon"
let kToolTipKey = "tooltip"
let kIdKey = "id"
let kTypeKey = "type"
let kLabelKey = "label"
let kSeparatorKey = "separator"
let kSubMenuKey = "submenu"
let kEnabledKey = "enabled"

let kChannelAppWindowName = "flutter/system_tray/app_window"

let kInitAppWindow = "InitAppWindow"
let kShowAppWindow = "ShowAppWindow"
let kHideAppWindow = "HideAppWindow"
let kCloseAppWindow = "CloseAppWindow"

public class SystemTrayPlugin: NSObject, FlutterPlugin {
  var system_tray: SystemTray?
  var app_window: AppWindow?
  var channel: FlutterMethodChannel
  var channel_app_window: FlutterMethodChannel
  var registrar: FlutterPluginRegistrar

  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: kChannelName, binaryMessenger: registrar.messenger)
    let channel_app_window = FlutterMethodChannel(
      name: kChannelAppWindowName, binaryMessenger: registrar.messenger)

    let instance = SystemTrayPlugin(registrar, channel, channel_app_window)

    registrar.addMethodCallDelegate(instance, channel: channel)
    registrar.addMethodCallDelegate(instance, channel: channel_app_window)
  }

  init(
    _ registrar: FlutterPluginRegistrar, _ channel: FlutterMethodChannel,
    _ channel_app_window: FlutterMethodChannel)
  {
    system_tray = SystemTray(channel)
    self.channel = channel
    self.channel_app_window = channel_app_window
    self.registrar = registrar
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    print("method: \(call.method)")

    switch call.method {
    case kInitSystemTray:
      initSystemTray(call, result)
    case kSetSystemTrayInfo:
      setSystemTrayInfo(call, result)
    case kSetContextMenu:
      setContextMenu(call, result)
    case kPopupContextMenu:
      popUpContextMenu(call, result)
    case kInitAppWindow:
      initAppWindow(call, result)
    case kShowAppWindow:
      showAppWindow(call, result)
    case kHideAppWindow:
      hideAppWindow(call, result)
    case kCloseAppWindow:
      closeAppWindow(call, result)
    default:
      result(FlutterMethodNotImplemented)
    }
  }

  func initSystemTray(_ call: FlutterMethodCall, _ result: FlutterResult) {
    let arguments = call.arguments as! [String: Any]
    let title = arguments[kTitleKey] as? String
    let base64Icon = arguments[kBase64IconKey] as? String
    let toolTip = arguments[kToolTipKey] as? String
    result(system_tray?.initSystemTray(title: title, base64Icon: base64Icon, toolTip: toolTip) ?? false)
  }

  func setSystemTrayInfo(_ call: FlutterMethodCall, _ result: FlutterResult) {
    let arguments = call.arguments as! [String: Any]
    let title = arguments[kTitleKey] as? String
    let base64Icon = arguments[kBase64IconKey] as? String
    let toolTip = arguments[kToolTipKey] as? String
    result(
      system_tray?.setSystemTrayInfo(title: title, base64Icon: base64Icon, toolTip: toolTip) ?? false)
  }

  func valueToMenuItem(menu: NSMenu, item: [String: Any]) -> Bool {
    let type = item[kTypeKey] as? String
    if type == nil {
      return false
    }

    let label = item[kLabelKey] as? String ?? ""
    let id = item[kIdKey] as? Int ?? -1
    let isEnabled = item[kEnabledKey] as? Bool ?? false

    switch type! {
    case kSeparatorKey:
      menu.addItem(.separator())
    case kSubMenuKey:
      let subMenu = NSMenu()
      let children = item[kSubMenuKey] as? [[String: Any]] ?? [[String: Any]]()
      if valueToMenu(menu: subMenu, items: children) {
        let menuItem = NSMenuItem()
        menuItem.title = label
        menuItem.submenu = subMenu
        menu.addItem(menuItem)
      }
    default:
      let menuItem = NSMenuItem()
      menuItem.title = label
      menuItem.target = self
      menuItem.action = isEnabled ? #selector(onMenuItemSelectedCallback) : nil
      menuItem.tag = id
      menu.addItem(menuItem)
    }

    return true
  }

  func valueToMenu(menu: NSMenu, items: [[String: Any]]) -> Bool {
    for item in items {
      if !valueToMenuItem(menu: menu, item: item) {
        return false
      }
    }
    return true
  }

  func setContextMenu(_ call: FlutterMethodCall, _ result: FlutterResult) {
    var popup_menu: NSMenu
    repeat {
      let items = call.arguments as? [[String: Any]]
      if items == nil {
        break
      }

      popup_menu = NSMenu()
      if !valueToMenu(menu: popup_menu, items: items!) {
        break
      }

      result(system_tray?.setContextMenu(menu: popup_menu) ?? false)
      return
    } while false

    result(false)
  }

  func popUpContextMenu(_ call: FlutterMethodCall, _ result: FlutterResult) {
    result(system_tray?.popUpContextMenu() ?? false)
  }

  @objc func onMenuItemSelectedCallback(_ sender: Any) {
    let menuItem = sender as! NSMenuItem
    channel.invokeMethod(kMenuItemSelectedCallbackMethod, arguments: menuItem.tag, result: nil)
  }

  func initAppWindow(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      if app_window != nil {
        result(true)
        break
      }

      let view = registrar.view
      if view == nil {
        break
      }

      let window = view!.window
      if window == nil {
        break
      }

      app_window = AppWindow(channel, window!)
      result(true)
      return
    } while false

    result(false)
  }

  func showAppWindow(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      if app_window == nil {
        break
      }

      app_window!.showAppWindow()
      result(true)
      return
    } while false

    result(false)
  }

  func hideAppWindow(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      if app_window == nil {
        break
      }

      app_window!.hideAppWindow()
      result(true)
      return
    } while false

    result(false)
  }

  func closeAppWindow(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      if app_window == nil {
        break
      }

      app_window!.closeAppWindow()
      result(true)
      return
    } while false

    result(false)
  }
}
