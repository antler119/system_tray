
import FlutterMacOS

let kDefaultSizeWidth = 18
let kDefaultSizeHeight = 18

let kSystemTrayEventCallbackMethod = "SystemTrayEventCallback";

let kSystemTrayEventLButtnUp = "leftMouseUp";
let kSystemTrayEventLButtonDblClk = "leftMouseDblClk";
let kSystemTrayEventRButtnUp = "rightMouseUp";

class SystemTray : NSObject, NSMenuDelegate {
    var statusItem: NSStatusItem?
    var statusItemMenu: NSMenu?
    var channel: FlutterMethodChannel

    init(_ channel: FlutterMethodChannel) {
        self.channel = channel
    }

    func menuDidClose(_ menu: NSMenu) {
        self.statusItem?.menu = nil
    }

    @objc func on_system_tray_event_callback(sender: NSStatusBarButton) {
        if let event = NSApp.currentEvent {
            switch event.type {
            case .leftMouseUp:
                channel.invokeMethod(kSystemTrayEventCallbackMethod, arguments: kSystemTrayEventLButtnUp)
            default:
                channel.invokeMethod(kSystemTrayEventCallbackMethod, arguments: kSystemTrayEventRButtnUp)

                statusItem?.menu = statusItemMenu
                statusItem?.button?.performClick(nil)
            }
        }
    }

    func init_system_tray(title: String?, iconPath: String?, toolTip: String?) -> Bool {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)

        statusItem?.button?.action = #selector(on_system_tray_event_callback(sender:))
        statusItem?.button?.target = self
        statusItem?.button?.sendAction(on: [.leftMouseUp, .rightMouseUp])

        if let toolTip = toolTip {
            statusItem?.button?.toolTip = toolTip
        }
        if let title = title {
            statusItem?.button?.title = title
        }

        if let iconPath = iconPath {
            if let itemImage = NSImage(named: iconPath) {
                let destSize = NSSize(width: kDefaultSizeWidth, height: kDefaultSizeHeight)
                itemImage.size = destSize
                statusItem?.button?.image = itemImage
            } 
        }
        return true
    }

    func set_system_tray_info(title: String?, iconPath: String?, toolTip: String?) -> Bool {
        if let toolTip = toolTip {
            statusItem?.button?.toolTip = toolTip
        }
        if let title = title {
            statusItem?.button?.title = title
        }
        if let iconPath = iconPath {
            if let itemImage = NSImage(named: iconPath) {
                let destSize = NSSize(width: kDefaultSizeWidth, height: kDefaultSizeHeight)
                itemImage.size = destSize
                statusItem?.button?.image = itemImage
            } else {
                statusItem?.button?.image = nil
            }
        }
 
        return true
    }

    func set_context_menu(menu: NSMenu) -> Bool {
        statusItemMenu = menu
        statusItemMenu?.delegate = self
        return true
    }


}