import FlutterMacOS

let kDefaultSizeWidth = 18
let kDefaultSizeHeight = 18

let kSystemTrayEventCallbackMethod = "SystemTrayEventCallback"

let kSystemTrayEventClick = "click"
let kSystemTrayEventRightClick = "right-click"
let kSystemTrayEventDoubleClick = "double-click"

class SystemTray: NSObject, NSMenuDelegate {
  var statusItem: NSStatusItem?
  var statusItemMenu: NSMenu?
  var channel: FlutterMethodChannel

  init(_ channel: FlutterMethodChannel) {
    self.channel = channel
  }

  func menuDidClose(_ menu: NSMenu) {
    statusItem?.menu = nil
  }

  @objc func onSystemTrayEventCallback(sender: NSStatusBarButton) {
    if let event = NSApp.currentEvent {
      switch event.type {
      case .leftMouseUp:
        channel.invokeMethod(kSystemTrayEventCallbackMethod, arguments: kSystemTrayEventClick)
      case .rightMouseUp:
        channel.invokeMethod(kSystemTrayEventCallbackMethod, arguments: kSystemTrayEventRightClick)
      default:
        break
      }
    }
  }

  func initSystemTray(title: String?, base64Icon: String?, toolTip: String?) -> Bool {
    statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)

    statusItem?.button?.action = #selector(onSystemTrayEventCallback(sender:))
    statusItem?.button?.target = self
    statusItem?.button?.sendAction(on: [
      .leftMouseUp, .leftMouseDown, .rightMouseUp, .rightMouseDown,
    ])

    if let toolTip = toolTip {
      statusItem?.button?.toolTip = toolTip
    }

    if let title = title {
      statusItem?.button?.title = title
    }

    if let base64Icon = base64Icon {
      if let imageData = Data(base64Encoded: base64Icon, options: .ignoreUnknownCharacters),
        let itemImage = NSImage(data: imageData)
      {
        let destSize = NSSize(width: kDefaultSizeWidth, height: kDefaultSizeHeight)
        itemImage.size = destSize
        statusItem?.button?.image = itemImage
        statusItem?.button?.imagePosition = NSControl.ImagePosition.imageLeft
      }
    }

    return true
  }

  func setSystemTrayInfo(title: String?, base64Icon: String?, toolTip: String?) -> Bool {
    if let toolTip = toolTip {
      statusItem?.button?.toolTip = toolTip
    }

    if let title = title {
      statusItem?.button?.title = title
    }

    if let base64Icon = base64Icon {
      if let imageData = Data(base64Encoded: base64Icon, options: .ignoreUnknownCharacters),
        let itemImage = NSImage(data: imageData)
      {
        let destSize = NSSize(width: kDefaultSizeWidth, height: kDefaultSizeHeight)
        itemImage.size = destSize
        statusItem?.button?.image = itemImage
        statusItem?.button?.imagePosition = NSControl.ImagePosition.imageLeft
      } else {
        statusItem?.button?.image = nil
      }
    }

    return true
  }

  func setContextMenu(menu: NSMenu) -> Bool {
    statusItemMenu = menu
    statusItemMenu?.delegate = self
    return true
  }

  func popUpContextMenu() -> Bool {
    if let statusItemMenu = statusItemMenu {
      statusItem?.menu = statusItemMenu
      statusItem?.button?.performClick(nil)
      return true
    }
    return false
  }

  func setPressedImage(base64Icon: String?) {
    if let base64Icon = base64Icon {
      if let imageData = Data(base64Encoded: base64Icon, options: .ignoreUnknownCharacters),
        let itemImage = NSImage(data: imageData)
      {
        let destSize = NSSize(width: kDefaultSizeWidth, height: kDefaultSizeHeight)
        itemImage.size = destSize
        statusItem?.button?.alternateImage = itemImage
        statusItem?.button?.setButtonType(.toggle)
      } else {
        statusItem?.button?.alternateImage = nil
      }
    } else {
      statusItem?.button?.alternateImage = nil
    }
  }

  func getTitle() -> String {
    return statusItem?.button?.title ?? ""
  }
}
