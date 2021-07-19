
let kDefaultSizeWidth = 18
let kDefaultSizeHeight = 18

class SystemTray {
    var statusItem: NSStatusItem?

    func init_system_tray(title: String, iconPath: String, toolTip: String) -> Bool {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        statusItem?.button?.toolTip = toolTip
        statusItem?.button?.title = title
        if let itemImage = NSImage(named: iconPath) {
            let destSize = NSSize(width: kDefaultSizeWidth, height: kDefaultSizeHeight)
            itemImage.size = destSize
            statusItem?.button?.image = itemImage
        } 
        return true
    }

    func set_context_menu(menu: NSMenu) -> Bool {
        statusItem?.menu = menu
        return true
    }
}