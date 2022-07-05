import Cocoa
import FlutterMacOS

let kCreateContextMenu = "CreateContextMenu"
let kSetLable = "SetLable"
let kSetImage = "SetImage"
let kSetEnable = "SetEnable"
let kSetCheck = "SetCheck"

private let kMenuIdKey = "menu_id"

class MenuManager: NSObject {
  var registrar: FlutterPluginRegistrar
  var channel: FlutterMethodChannel
  var menuMap = [Int: Menu]()

  init(_ registrar: FlutterPluginRegistrar, _ channel: FlutterMethodChannel) {
    self.registrar = registrar
    self.channel = channel
  }

  func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case kCreateContextMenu:
      createContextMenu(call, result)
    case kSetLable:
      setLable(call, result)
    case kSetImage:
      setImage(call, result)
    case kSetEnable:
      setEnable(call, result)
    case kSetCheck:
      setCheck(call, result)
    default:
      result(FlutterMethodNotImplemented)
    }
  }

  func createContextMenu(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let arguments = call.arguments as! [String: Any]
      let menuId = arguments[kMenuIdKey] as? Int

      if menuId == nil {
        break
      }

      let menu = Menu(self.channel, menuId!)
      if !menu.createContextMenu(call) {
        break
      }

      if !addMenu(menuId: menuId!, menu: menu) {
        break
      }

      result(true)
      return
    } while false

    result(false)
  }

  func setLable(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let menu = getMenu(call)
      if menu == nil {
        break
      }

      menu!.setLable(call, result)
      return
    } while false

    result(false)
  }

  func setImage(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let menu = getMenu(call)
      if menu == nil {
        break
      }

      menu!.setImage(call, result)
      return
    } while false

    result(false)
  }

  func setEnable(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let menu = getMenu(call)
      if menu == nil {
        break
      }

      menu!.setEnable(call, result)
      return
    } while false

    result(false)
  }

  func setCheck(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let menu = getMenu(call)
      if menu == nil {
        break
      }

      menu!.setCheck(call, result)
      return
    } while false

    result(false)
  }

  func addMenu(menuId: Int, menu: Menu) -> Bool {
    menuMap[menuId] = menu
    return true
  }

  func getMenu(menuId: Int) -> Menu? {
    return menuMap[menuId]
  }

  func getMenu(_ call: FlutterMethodCall) -> Menu? {
    var menu: Menu?
    repeat {
      let arguments = call.arguments as! [String: Any]
      let menuId = arguments[kMenuIdKey] as? Int

      if menuId == nil {
        break
      }

      menu = getMenu(menuId: menuId!)

    } while false

    return menu
  }
}
