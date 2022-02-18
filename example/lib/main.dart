import 'dart:async';
import 'dart:io';
import 'dart:math';

import 'package:bitsdojo_window/bitsdojo_window.dart';
import 'package:english_words/english_words.dart';
import 'package:flutter/material.dart';
import 'package:system_tray/system_tray.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const MyApp());

  doWhenWindowReady(() {
    final win = appWindow;
    const initialSize = Size(600, 450);
    win.minSize = initialSize;
    win.size = initialSize;
    win.alignment = Alignment.center;
    win.title = "How to use system tray with Flutter";
    win.show();
  });
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final SystemTray _systemTray = SystemTray();
  final AppWindow _appWindow = AppWindow();
  late List<MenuItemBase> _trayMenu;

  Timer? _timer;
  bool _toogleTrayIcon = true;

  @override
  void initState() {
    super.initState();
    initSystemTray();
  }

  @override
  void dispose() {
    super.dispose();
    _timer?.cancel();
  }

  Future<void> initSystemTray() async {
    String path =
        Platform.isWindows ? 'assets/app_icon.ico' : 'assets/app_icon.png';

    List<String> iconList = ['darts_icon', 'gift_icon'];

    _trayMenu = [
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
              _systemTray.setImage(_toogleTrayIcon ? "" : path);
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

          _systemTray.setImage(path);
        },
      ),
      MenuSeparator(),
      SubMenu(
        label: "Test API",
        children: [
          SubMenu(
            label: "setSystemTrayInfo",
            children: [
              MenuItem(
                label: 'setTitle',
                onClicked: () {
                  final String text = WordPair.random().asPascalCase;
                  debugPrint("click 'setTitle' : $text");
                  _systemTray.setTitle(text);
                },
              ),
              MenuItem(
                label: 'setImage',
                onClicked: () {
                  String iconName = iconList[Random().nextInt(iconList.length)];
                  String path = Platform.isWindows
                      ? 'assets/$iconName.ico'
                      : 'assets/$iconName.png';

                  debugPrint("click 'setImage' : $path");
                  _systemTray.setImage(path);
                },
              ),
              MenuItem(
                label: 'setToolTip',
                onClicked: () {
                  final String text = WordPair.random().asPascalCase;
                  debugPrint("click 'setToolTip' : $text");
                  _systemTray.setToolTip(text);
                },
              ),
              MenuItem(
                label: 'getTitle [macOS]',
                onClicked: () async {
                  String title = await _systemTray.getTitle();
                  debugPrint("click 'getTitle' : $title");
                },
              ),
            ],
          ),
          MenuItem(label: 'disabled Item', enabled: false),
        ],
      ),
      MenuSeparator(),
      MenuCheckbox(
        label: 'Checkbox 1',
        checked: true,
        onClicked: () {
          debugPrint("click 'Checkbox 1'");
        },
      ),
      MenuCheckbox(
        label: 'Checkbox 2',
        onClicked: () {
          debugPrint("click 'Checkbox 2'");
        },
      ),
      MenuCheckbox(
        label: 'Checkbox 3',
        checked: true,
        onClicked: () {
          debugPrint("click 'Checkbox 3'");

          (_trayMenu.elementAt(9) as MenuCheckbox).checked =
              !(_trayMenu.elementAt(9) as MenuCheckbox).checked;

          _systemTray.setContextMenu(_trayMenu);
        },
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

    await _systemTray.setContextMenu(_trayMenu);

    // handle system tray event
    _systemTray.registerSystemTrayEventHandler((eventName) {
      debugPrint("eventName: $eventName");
      if (eventName == kSystemTrayEventClick) {
        Platform.isWindows ? _appWindow.show() : _systemTray.popUpContextMenu();
      } else if (eventName == kSystemTrayEventRightClick) {
        Platform.isWindows ? _systemTray.popUpContextMenu() : _appWindow.show();
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: Scaffold(
        body: WindowBorder(
          color: const Color(0xFF805306),
          width: 1,
          child: Row(
            children: const [
              LeftSide(),
              RightSide(),
            ],
          ),
        ),
      ),
    );
  }
}

const backgroundStartColor = Color(0xFFFFD500);
const backgroundEndColor = Color(0xFFF6A00C);

class LeftSide extends StatelessWidget {
  const LeftSide({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 200,
      child: Container(
        color: const Color(0xFFFFFFFF),
        child: Column(
          children: [
            WindowTitleBarBox(
              child: Container(
                decoration: const BoxDecoration(
                  gradient: LinearGradient(
                      begin: Alignment.topCenter,
                      end: Alignment.bottomCenter,
                      colors: [backgroundStartColor, backgroundEndColor],
                      stops: [0.0, 1.0]),
                ),
                child: MoveWindow(),
              ),
            ),
            Expanded(
              child: Container(),
            )
          ],
        ),
      ),
    );
  }
}

class RightSide extends StatelessWidget {
  const RightSide({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: Container(
        color: const Color(0xFFFFFFFF),
        child: Column(
          children: [
            WindowTitleBarBox(
              child: Container(
                decoration: const BoxDecoration(
                  gradient: LinearGradient(
                      begin: Alignment.topCenter,
                      end: Alignment.bottomCenter,
                      colors: [backgroundStartColor, backgroundEndColor],
                      stops: [0.0, 1.0]),
                ),
                child: Row(
                  children: [
                    Expanded(
                      child: MoveWindow(),
                    ),
                    const WindowButtons()
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

final buttonColors = WindowButtonColors(
    iconNormal: const Color(0xFF805306),
    mouseOver: const Color(0xFFF6A00C),
    mouseDown: const Color(0xFF805306),
    iconMouseOver: const Color(0xFF805306),
    iconMouseDown: const Color(0xFFFFD500));

final closeButtonColors = WindowButtonColors(
    mouseOver: const Color(0xFFD32F2F),
    mouseDown: const Color(0xFFB71C1C),
    iconNormal: const Color(0xFF805306),
    iconMouseOver: Colors.white);

class WindowButtons extends StatelessWidget {
  const WindowButtons({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        MinimizeWindowButton(colors: buttonColors),
        MaximizeWindowButton(colors: buttonColors),
        CloseWindowButton(colors: closeButtonColors),
      ],
    );
  }
}
