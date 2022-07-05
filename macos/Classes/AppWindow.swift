import FlutterMacOS

let kInitAppWindow = "InitAppWindow"
let kShowAppWindow = "ShowAppWindow"
let kHideAppWindow = "HideAppWindow"
let kCloseAppWindow = "CloseAppWindow"

class AppWindow: NSObject {
  var registrar: FlutterPluginRegistrar
  var channel: FlutterMethodChannel

  var window: NSWindow?

  init(_ registrar: FlutterPluginRegistrar, _ channel: FlutterMethodChannel) {
    self.registrar = registrar
    self.channel = channel
  }

  func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
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

  func initAppWindow(_ call: FlutterMethodCall, _ result: FlutterResult) {
    repeat {
      let view = self.registrar.view
      if view == nil {
        break
      }

      let window = view!.window
      if window == nil {
        break
      }

      self.window = window
      result(true)
      return
    } while false

    result(false)
  }

  func showAppWindow(_ call: FlutterMethodCall, _ result: @escaping FlutterResult) {
    // print("showAppWindow title:\(self.window.title) rect:\(self.window.contentLayoutRect)")
    DispatchQueue.main.async {
      if self.window?.isMiniaturized == true {
        self.window?.deminiaturize(self)
      } else {
        self.window?.windowController?.showWindow(self)
        NSApp.activate(ignoringOtherApps: true)
        self.window?.makeKeyAndOrderFront(self)
      }
    }

    result(true)
  }

  func hideAppWindow(_ call: FlutterMethodCall, _ result: @escaping FlutterResult) {
    DispatchQueue.main.async {
      self.window?.miniaturize(self)
    }

    result(true)
  }

  func closeAppWindow(_ call: FlutterMethodCall, _ result: FlutterResult) {
    self.window?.performClose(self)

    result(true)
  }
}
