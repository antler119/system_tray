import Cocoa
import FlutterMacOS

let kInitSystemTray = "InitSystemTray"
let kSetSystemTrayInfo = "SetSystemTrayInfo"
let kSetContextMenu = "SetContextMenu"
let kPopupContextMenu = "PopupContextMenu"
let kSetPressedImage = "SetPressedImage"
let kGetTitle = "GetTitle"
let kDestroySystemTray = "DestroySystemTray"

private let kDefaultSizeWidth = 18
private let kDefaultSizeHeight = 18

private let kTitleKey = "title"
private let kIconPathKey = "iconpath"
private let kToolTipKey = "tooltip"
private let kIsTemplateKey = "is_template"
private let kFontFamilyKey = "font_family"
private let kPreferMonospacedFallbackKey = "prefer_monospaced_fallback"

private let kSystemTrayEventClick = "click"
private let kSystemTrayEventRightClick = "right-click"
private let kSystemTrayEventDoubleClick = "double-click"

private let kSystemTrayEventCallbackMethod = "SystemTrayEventCallback"

class Tray: NSObject, NSMenuDelegate {
  var registrar: FlutterPluginRegistrar
  var channel: FlutterMethodChannel

  weak var menuManager: MenuManager?

  var statusItem: NSStatusItem?

  var contextMenuId: Int?

  init(
    _ registrar: FlutterPluginRegistrar, _ channel: FlutterMethodChannel,
    _ menuManager: MenuManager?
  ) {
    self.registrar = registrar
    self.channel = channel
    self.menuManager = menuManager
  }

  func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case kInitSystemTray:
      initSystemTray(call, result)
    case kSetSystemTrayInfo:
      setTrayInfo(call, result)
    case kSetContextMenu:
      setContextMenu(call, result)
    case kPopupContextMenu:
      popUpContextMenu(call, result)
    case kSetPressedImage:
      setPressedImage(call, result)
    case kGetTitle:
      getTitle(call, result)
    case kDestroySystemTray:
      destroySystemTray(call, result)
    default:
      result(FlutterMethodNotImplemented)
    }
  }

  func menuDidClose(_ menu: NSMenu) {
    statusItem?.menu = nil
  }

  /// Sets the title. If fontFamily is provided, uses NSAttributedString.
  /// Otherwise, falls back to standard system behavior.
  private func updateTitle(_ title: String, fontFamily: String?, preferMonospacedFallback: Bool = false) {
    guard let button = statusItem?.button else { return }

    // Case 1: No font family requested. Use standard behavior.
    guard let family = fontFamily, !family.isEmpty else {
      button.title = title
      button.attributedTitle = nil // Clear any previous attributes
      return
    }

    // Case 2: User requested a custom font
    let fontSize: CGFloat = 13.0
    var font: NSFont?

    // Check for special "system_monospaced" keyword
    if family == "system_monospaced" {
      if #available(macOS 10.15, *) {
        font = NSFont.monospacedSystemFont(ofSize: fontSize, weight: .regular)
      } else {
        font = NSFont.userFixedPitchFont(ofSize: fontSize)
      }
    } else {
      // Try to load the custom font by name (e.g., "Menlo", "Arial", "Fira Code")
      // Note: Use the PostScript name or family name as shown in Font Book
      font = NSFont(name: family, size: fontSize)
      
      // If custom font fails to load, use fallback based on user preference
      if font == nil {
        if preferMonospacedFallback {
          // Fallback to monospaced font
          if #available(macOS 10.15, *) {
            font = NSFont.monospacedSystemFont(ofSize: fontSize, weight: .regular)
          } else {
            font = NSFont.userFixedPitchFont(ofSize: fontSize)
          }
        } else {
          // Fallback to standard system font (proportional)
          font = NSFont.systemFont(ofSize: fontSize)
        }
      }
    }

    // Apply the font (either custom, system_monospaced, or fallback)
    if let validFont = font {
      let attributes: [NSAttributedString.Key: Any] = [.font: validFont]
      button.attributedTitle = NSAttributedString(string: title, attributes: attributes)
    } else {
      // This should rarely happen, but if all font loading fails, use standard title
      button.title = title
      button.attributedTitle = nil
    }
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

  func initSystemTray(_ call: FlutterMethodCall, _ result: FlutterResult) {
    let arguments = call.arguments as! [String: Any]
    let title = arguments[kTitleKey] as? String
    let base64Icon = arguments[kIconPathKey] as? String
    let toolTip = arguments[kToolTipKey] as? String
    let isTemplate = arguments[kIsTemplateKey] as? Bool
    let fontFamily = arguments[kFontFamilyKey] as? String // Extract font family
    let preferMonospacedFallback = arguments[kPreferMonospacedFallbackKey] as? Bool ?? false

    if statusItem != nil {
      result(false)
      return
    }

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
      // Use the new update helper
      updateTitle(title, fontFamily: fontFamily, preferMonospacedFallback: preferMonospacedFallback)
    }

    if let base64Icon = base64Icon {
      if let imageData = Data(base64Encoded: base64Icon, options: .ignoreUnknownCharacters),
        let itemImage = NSImage(data: imageData)
      {
        let destSize = NSSize(width: kDefaultSizeWidth, height: kDefaultSizeHeight)
        itemImage.size = destSize
        itemImage.isTemplate = isTemplate ?? false
        statusItem?.button?.image = itemImage
        statusItem?.button?.imagePosition = NSControl.ImagePosition.imageLeft
      }
    }

    result(true)
  }

  func setTrayInfo(_ call: FlutterMethodCall, _ result: FlutterResult) {
    let arguments = call.arguments as! [String: Any]
    let title = arguments[kTitleKey] as? String
    let base64Icon = arguments[kIconPathKey] as? String
    let toolTip = arguments[kToolTipKey] as? String
    let isTemplate = arguments[kIsTemplateKey] as? Bool
    let fontFamily = arguments[kFontFamilyKey] as? String // Extract font family
    let preferMonospacedFallback = arguments[kPreferMonospacedFallbackKey] as? Bool ?? false

    if let toolTip = toolTip {
      statusItem?.button?.toolTip = toolTip
    }

    if let title = title {
      // Use the new update helper
      updateTitle(title, fontFamily: fontFamily, preferMonospacedFallback: preferMonospacedFallback)
    }

    if let base64Icon = base64Icon {
      if let imageData = Data(base64Encoded: base64Icon, options: .ignoreUnknownCharacters),
        let itemImage = NSImage(data: imageData)
      {
        let destSize = NSSize(width: kDefaultSizeWidth, height: kDefaultSizeHeight)
        itemImage.size = destSize
        itemImage.isTemplate = isTemplate ?? false
        statusItem?.button?.image = itemImage
        statusItem?.button?.imagePosition = NSControl.ImagePosition.imageLeft
      } else {
        statusItem?.button?.image = nil
      }
    }

    return result(true)
  }

  func setContextMenu(_ call: FlutterMethodCall, _ result: FlutterResult) {
    if let menuId = call.arguments as? Int {
      contextMenuId = menuId
    } else {
      contextMenuId = -1
    }
    result(true)
  }

  func popUpContextMenu(_ call: FlutterMethodCall, _ result: FlutterResult) {
    if let menu = menuManager?.getMenu(menuId: contextMenuId ?? -1) {
      let nsMenu = menu.getNSMenu()
      nsMenu?.delegate = self

      statusItem?.menu = nsMenu
      statusItem?.button?.performClick(nil)
      result(true)
      return
    }
    result(false)
  }

  func setPressedImage(_ call: FlutterMethodCall, _ result: FlutterResult) {
    let base64Icon = call.arguments as? String

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

    result(nil)
  }

  func getTitle(_ call: FlutterMethodCall, _ result: FlutterResult) {
    result(statusItem?.button?.title ?? "")
  }

  func destroySystemTray(_ call: FlutterMethodCall, _ result: FlutterResult) {
    contextMenuId = -1

    if statusItem != nil {
      NSStatusBar.system.removeStatusItem(statusItem!)

      statusItem?.button?.image = nil
      statusItem?.button?.alternateImage = nil
      statusItem = nil
    }
    result(true)
  }
}
