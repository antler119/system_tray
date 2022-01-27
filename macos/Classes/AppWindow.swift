import FlutterMacOS

class AppWindow: NSObject {
  var channel: FlutterMethodChannel
  var window: NSWindow

  init(_ channel: FlutterMethodChannel, _ window: NSWindow) {
    self.channel = channel
    self.window = window
  }

  func showAppWindow() {
    print("showAppWindow title:\(self.window.title) rect:\(self.window.contentLayoutRect)")
    DispatchQueue.main.async {
      if self.window.isMiniaturized == true {
        self.window.deminiaturize(self)
      } else {
        self.window.windowController?.showWindow(self)
        NSApp.activate(ignoringOtherApps: true)
        self.window.makeKeyAndOrderFront(self)
      }
    }
  }

  func hideAppWindow() {
    print("hideAppWindow")
    DispatchQueue.main.async {
      self.window.miniaturize(self)
    }
  }

  func closeAppWindow() {
    print("closeAppWindow")
    self.window.performClose(self)
  }
}
