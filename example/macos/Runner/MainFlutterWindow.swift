import Cocoa
import FlutterMacOS
import bitsdojo_window_macos

// extension FlutterViewController {
//   override open func viewWillAppear() {
//     super.viewWillAppear()
//   }
  
//   override open func viewWillDisappear() {
//     super.viewWillDisappear()
//   }  
// }

class MainFlutterWindow: BitsdojoWindow {
  override func bitsdojo_window_configure() -> UInt {
    return BDW_CUSTOM_FRAME | BDW_HIDE_ON_STARTUP
  }

  override func awakeFromNib() {
    let flutterViewController = FlutterViewController.init()
    let windowFrame = self.frame
    self.contentViewController = flutterViewController
    self.setFrame(windowFrame, display: true)

    RegisterGeneratedPlugins(registry: flutterViewController)

    super.awakeFromNib()
  }
}
