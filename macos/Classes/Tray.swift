
class SystemTray {
    var statusItem: NSStatusItem?

    func init_system_tray(title: String, iconPath: String) -> Bool {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        statusItem?.button?.toolTip = title
        statusItem?.button?.title = "WorldTime"
        return true
    }

    func set_context_menu(menu: NSMenu) -> Bool {
        statusItem?.menu = menu
        return true
    }
}