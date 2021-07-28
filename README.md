# system_tray

A [Flutter package](https://github.com/antler119/system_tray.git) that that enables support for system tray menu for desktop flutter apps. **on Windows, macOS and Linux**. 

**Features**:

    - Modify system tray title/icon/tooltip
    - Handle system tray event leftMouseUp/rightMouseUp (only for macos、windows)

# Getting Started

Install the package using `pubspec.yaml`

# For Windows
<img src="https://raw.githubusercontent.com/antler119/system_tray/master/resources/screenshot_windows.jpg">

# For macOS
<img src="https://raw.githubusercontent.com/antler119/system_tray/master/resources/screenshot_macos.jpg">

# For Linux
<img src="https://raw.githubusercontent.com/antler119/system_tray/master/resources/screenshot_ubuntu.jpg">

## API

Below we show how to use system_tray
```dart
Future<void> initSystemTray() async {
    String path;
    if (Platform.isWindows) {
      path = p.joinAll([
        p.dirname(Platform.resolvedExecutable),
        'data/flutter_assets/assets',
        'app_icon.ico'
      ]);
    } else if (Platform.isMacOS) {
      path = p.joinAll(['AppIcon']);
    } else {
      path = p.joinAll([
        p.dirname(Platform.resolvedExecutable),
        'data/flutter_assets/assets',
        'app_icon.png'
      ]);
    }

    // We first init the systray menu and then add the menu entries
    await _systemTray.initSystemTray("system tray",
        iconPath: path, toolTip: "How to use system tray with Flutter");

    await _systemTray.setContextMenu(
      [
        MenuItem(
          label: 'Show',
          onClicked: () {
            appWindow.show();
          },
        ),
        MenuSeparator(),
        SubMenu(
          label: "SubMenu",
          children: [
            MenuItem(
              label: 'SubItem1',
              enabled: false,
              onClicked: () {
                print("click SubItem1");
              },
            ),
            MenuItem(label: 'SubItem2'),
            MenuItem(label: 'SubItem3'),
          ],
        ),
        MenuSeparator(),
        MenuItem(
          label: 'Exit',
          onClicked: () {
            appWindow.close();
          },
        ),
      ],
    );

    // flash tray icon
    _timer = Timer.periodic(
      const Duration(milliseconds: 500),
      (timer) {
        _toogleTrayIcon = !_toogleTrayIcon;
        _systemTray.setSystemTrayInfo(
          iconPath: _toogleTrayIcon ? "" : path,
        );
      },
    );

    // handle system tray event
    _systemTray.registerSystemTrayEventHandler((eventName) {
      print("eventName: $eventName");
    });
  }
```