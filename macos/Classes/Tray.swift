
class SystemTray {
    var statusItem: NSStatusItem?

    func init_system_tray(title: String, iconPath: String, toolTip: String) -> Bool {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        statusItem?.button?.toolTip = toolTip
        statusItem?.button?.title = title
        return true
    }

    func set_context_menu(menu: NSMenu) -> Bool {
        statusItem?.menu = menu
        return true
    }
}