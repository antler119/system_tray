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

1. Smallest example

```dart
Future<void> initSystemTray() async {
  final path =
      Platform.isWindows ? 'assets/app_icon.ico' : 'assets/app_icon.png';
  final menu = [
    MenuItem(label: 'Show', onClicked: _appWindow.show),
    MenuItem(label: 'Hide', onClicked: _appWindow.hide),
    MenuItem(label: 'Exit', onClicked: _appWindow.close),
  ];

  await _systemTray.initSystemTray(
    title: "system tray",
    iconPath: path,
  );

  await _systemTray.setContextMenu(menu);
}
```

2. Icon flashing effect example

```dart
Future<void> initSystemTray() async {
  final path =
      Platform.isWindows ? 'assets/app_icon.ico' : 'assets/app_icon.png';
  final menu = [
    MenuItem(label: 'Show', onClicked: _appWindow.show),
    MenuItem(label: 'Hide', onClicked: _appWindow.hide),
    MenuItem(
      label: 'Start flash tray icon',
      onClicked: () {
        debugPrint("Start flash tray icon");

        _timer ??= Timer.periodic(
          const Duration(milliseconds: 500),
          (timer) {
            _toogleTrayIcon = !_toogleTrayIcon;
            _systemTray.setSystemTrayInfo(
              iconPath: _toogleTrayIcon ? "" : path,
            );
          },
        );
      },
    ),
    MenuItem(
      label: 'Stop flash tray icon',
      onClicked: () {
        debugPrint("Stop flash tray icon");

        _timer?.cancel();
        _timer = null;

        _systemTray.setSystemTrayInfo(
          iconPath: path,
        );
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
            debugPrint("click SubItem1");
          },
        ),
        MenuItem(
          label: 'SubItem2',
          onClicked: () {
            debugPrint("click SubItem2");
          },
        ),
        MenuItem(
          label: 'SubItem3',
          onClicked: () {
            debugPrint("click SubItem3");
          },
        ),
      ],
    ),
    MenuSeparator(),
    MenuItem(
      label: 'Exit',
      onClicked: _appWindow.close,
    ),
  ];

  // We first init the systray menu and then add the menu entries
  await _systemTray.initSystemTray(
    title: "system tray",
    iconPath: path,
    toolTip: "How to use system tray with Flutter",
  );

  await _systemTray.setContextMenu(menu);

  // handle system tray event
  _systemTray.registerSystemTrayEventHandler((eventName) {
    debugPrint("eventName: $eventName");
    if (eventName == "leftMouseUp") {
      _appWindow.show();
    }
  });
}
```

# Addition

Recommended library that supports window control:

- [bitsdojo_window](https://pub.dev/packages/bitsdojo_window)
- [window_size (Google)](https://github.com/google/flutter-desktop-embedding/tree/master/plugins/window_size)
