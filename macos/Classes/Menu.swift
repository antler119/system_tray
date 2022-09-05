import FlutterMacOS

private let kDefaultIconSizeWidth = 18
private let kDefaultIconSizeHeight = 18

private let kMenuIdKey = "menu_id"
private let kMenuItemIdKey = "menu_item_id"
private let kMenuListKey = "menu_list"
private let kIdKey = "id"
private let kTypeKey = "type"
private let kCheckboxKey = "checkbox"
private let kSeparatorKey = "separator"
private let kSubMenuKey = "submenu"
private let kLabelKey = "label"
private let kImageKey = "image"
private let kEnabledKey = "enabled"
private let kCheckedKey = "checked"

private let kMenuItemSelectedCallbackMethod = "MenuItemSelectedCallback"

class Menu: NSObject {
  var channel: FlutterMethodChannel
  var menuId: Int
  var nsMenu: NSMenu?

  init(_ channel: FlutterMethodChannel, _ menuId: Int) {
    self.channel = channel
    self.menuId = menuId
  }

  func createContextMenu(_ call: FlutterMethodCall) -> Bool {
    var result = false

    repeat {
      let arguments = call.arguments as! [String: Any]
      let menuList = arguments[kMenuListKey] as? [[String: Any]]

      if menuList == nil {
        break
      }

      if !createContextMenu(menuList!) {
        break
      }

      result = true
    } while false

    return result
  }

  func setLabel(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let arguments = call.arguments as! [String: Any]
      let menuItemId = arguments[kMenuItemIdKey] as? Int
      let label = arguments[kLabelKey] as? String

      if menuItemId == nil || label == nil {
        break
      }

      setLabel(menuItemId: menuItemId!, label: label!)

      result(true)
      return
    } while false

    result(false)
  }

  func setImage(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let arguments = call.arguments as! [String: Any]
      let menuItemId = arguments[kMenuItemIdKey] as? Int
      let image = arguments[kImageKey] as? String

      if menuItemId == nil {
        break
      }

      setImage(menuItemId: menuItemId!, base64Icon: image)

      result(true)
      return
    } while false

    result(false)
  }

  func setEnable(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let arguments = call.arguments as! [String: Any]
      let menuItemId = arguments[kMenuItemIdKey] as? Int
      let enabled = arguments[kEnabledKey] as? Bool

      if menuItemId == nil || enabled == nil {
        break
      }

      setEnable(menuItemId: menuItemId!, enabled: enabled!)

      result(true)
      return
    } while false

    result(false)
  }

  func setCheck(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let arguments = call.arguments as! [String: Any]
      let menuItemId = arguments[kMenuItemIdKey] as? Int
      let checked = arguments[kCheckedKey] as? Bool

      if menuItemId == nil || checked == nil {
        break
      }

      setCheck(menuItemId: menuItemId!, checked: checked!)

      result(true)
      return
    } while false

    result(false)
  }

  func createContextMenu(_ items: [[String: Any]]) -> Bool {
    repeat {
      let nsMenu = NSMenu()
      if !valueToMenu(menu: nsMenu, items: items) {
        break
      }

      self.nsMenu = nsMenu
      return true
    } while false

    return false
  }

  func getNSMenu() -> NSMenu? {
    return nsMenu
  }

  func setLabel(menuItemId: Int, label: String) {
    self.nsMenu?.item(withTag: menuItemId)?.title = label
  }

  func setImage(menuItemId: Int, base64Icon: String?) {
    var image: NSImage?
    if let base64Icon = base64Icon {
      if let imageData = Data(base64Encoded: base64Icon, options: .ignoreUnknownCharacters),
        let itemImage = NSImage(data: imageData)
      {
        itemImage.size = NSSize(width: kDefaultIconSizeWidth, height: kDefaultIconSizeHeight)
        image = itemImage
      }
    }

    self.nsMenu?.item(withTag: menuItemId)?.image = image
  }

  func setEnable(menuItemId: Int, enabled: Bool) {
    self.nsMenu?.item(withTag: menuItemId)?.isEnabled = enabled
  }

  func setCheck(menuItemId: Int, checked: Bool) {
    self.nsMenu?.item(withTag: menuItemId)?.state = checked ? .on : .off
  }

  func valueToMenu(menu: NSMenu, items: [[String: Any]]) -> Bool {
    for item in items {
      if !valueToMenuItem(menu: menu, item: item) {
        return false
      }
    }
    return true
  }

  func valueToMenuItem(menu: NSMenu, item: [String: Any]) -> Bool {
    let type = item[kTypeKey] as? String
    if type == nil {
      return false
    }

    let label = item[kLabelKey] as? String ?? ""
    let id = item[kIdKey] as? Int ?? -1

    var image: NSImage?
    if let base64Icon = item[kImageKey] as? String {
      if let imageData = Data(base64Encoded: base64Icon, options: .ignoreUnknownCharacters),
        let itemImage = NSImage(data: imageData)
      {
        itemImage.size = NSSize(width: kDefaultIconSizeWidth, height: kDefaultIconSizeHeight)
        image = itemImage
      }
    }

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
        menuItem.image = image
        menuItem.submenu = subMenu
        menu.addItem(menuItem)
      }
    case kCheckboxKey:
      let isChecked = item[kCheckedKey] as? Bool ?? false

      let menuItem = NSMenuItem()
      menuItem.title = label
      menuItem.image = image
      menuItem.target = self
      menuItem.action = isEnabled ? #selector(onMenuItemSelectedCallback) : nil
      menuItem.tag = id
      menuItem.state = isChecked ? .on : .off
      menu.addItem(menuItem)
    default:
      let menuItem = NSMenuItem()
      menuItem.title = label
      menuItem.image = image
      menuItem.target = self
      menuItem.action = isEnabled ? #selector(onMenuItemSelectedCallback) : nil
      menuItem.tag = id
      menu.addItem(menuItem)
    }

    return true
  }

  @objc func onMenuItemSelectedCallback(_ sender: Any) {
    let menuItem = sender as! NSMenuItem
    channel.invokeMethod(
      kMenuItemSelectedCallbackMethod,
      arguments: [kMenuIdKey: menuId, kMenuItemIdKey: menuItem.tag],
      result: nil)
  }
}
