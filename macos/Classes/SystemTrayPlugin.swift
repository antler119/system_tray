import Cocoa
import FlutterMacOS

let kChannelName = "flutter/system_tray"
let kInitSystemTray = "InitSystemTray";
let kSetSystemTrayInfo = "SetSystemTrayInfo";
let kSetContextMenu = "SetContextMenu";
let kMenuItemSelectedCallbackMethod = "MenuItemSelectedCallback";

let kTitleKey = "title"
let kIconPathKey = "iconpath"
let kToolTipKey = "tooltip"
let kIdKey = "id";
let kTypeKey = "type";
let kLabelKey = "label";
let kSeparatorKey = "separator";
let kSubMenuKey = "submenu";
let kEnabledKey = "enabled";

public class SystemTrayPlugin: NSObject, FlutterPlugin {
  var system_tray: SystemTray?
  var channel: FlutterMethodChannel

  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: kChannelName, binaryMessenger: registrar.messenger)
    let instance = SystemTrayPlugin(channel)
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  init(_ channel: FlutterMethodChannel) {
    system_tray = SystemTray()
    self.channel = channel
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case kInitSystemTray:
      init_system_tray(call, result)
    case kSetSystemTrayInfo:
      set_system_tray_info(call, result)
    case kSetContextMenu:
      set_context_menu(call, result)
    default:
      result(FlutterMethodNotImplemented)
    }
  }

  func init_system_tray(_ call: FlutterMethodCall, _ result: FlutterResult) {
      let arguments = call.arguments as! [String: Any]
      let title = arguments[kTitleKey] as? String
      let iconPath = arguments[kIconPathKey] as? String
      let toolTip = arguments[kToolTipKey] as? String
      result(system_tray?.init_system_tray(title: title, iconPath: iconPath, toolTip: toolTip) ?? false)
  }

  func set_system_tray_info(_ call: FlutterMethodCall, _ result: FlutterResult) {
      let arguments = call.arguments as! [String: Any]
      let title = arguments[kTitleKey] as? String
      let iconPath = arguments[kIconPathKey] as? String
      let toolTip = arguments[kToolTipKey] as? String
      result(system_tray?.set_system_tray_info(title: title, iconPath: iconPath, toolTip: toolTip) ?? false)
  }


  func value_to_menu_item(menu: NSMenu, item: [String: Any]) -> Bool {
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
      if value_to_menu(menu: subMenu, items: children) {
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

  func value_to_menu(menu: NSMenu, items: [[String: Any]]) -> Bool {
      for item in items {
        if !value_to_menu_item(menu: menu, item: item) {
            return false
        }
      }
      return true
  }

  func set_context_menu(_ call: FlutterMethodCall, _ result: FlutterResult) {
      var popup_menu: NSMenu
      repeat {
        let items = call.arguments as? [[String: Any]]
        if items == nil {
          break
        }

        popup_menu = NSMenu()
        if !value_to_menu(menu: popup_menu, items: items!) {
          break
        }
 
        result(system_tray?.set_context_menu(menu: popup_menu) ?? false)
        return
      } while false

      result(false)
  }

  @objc func onMenuItemSelectedCallback(_ sender:Any) {
    let menuItem = sender as! NSMenuItem
    channel.invokeMethod(kMenuItemSelectedCallbackMethod, arguments: menuItem.tag)
  }
}
