import FlutterMacOS

class AppWindow : NSObject{
    var channel: FlutterMethodChannel
    var window: NSWindow

    init(_ channel: FlutterMethodChannel, _ window: NSWindow) {
        self.channel = channel
        self.window = window
    } 

    func showAppWindow() {
        print("showAppWindow title:\(self.window.title) rect:\(self.window.contentLayoutRect)");
        self.window.windowController?.showWindow(self)
        NSApp.activate(ignoringOtherApps:true)
        self.window.orderFront(self)
    }

    func hideAppWindow() {
        print("hideAppWindow");
        self.window.orderOut(self)
    }

    func closeAppWindow() {
        print("closeAppWindow");
        self.window.performClose(self)
    }
}