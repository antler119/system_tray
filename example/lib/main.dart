import 'dart:async';
import 'dart:io';
import 'dart:math';

import 'package:bitsdojo_window/bitsdojo_window.dart';
import 'package:english_words/english_words.dart';
import 'package:flutter/material.dart' hide MenuItem;
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
  final AppWindow _appWindow = AppWindow();
  final SystemTray _systemTray = SystemTray();
  final Menu _menuMain = Menu();
  final Menu _menuSimple = Menu();

  Timer? _timer;
  bool _toogleTrayIcon = true;

  bool _toogleMenu = true;

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

  String getImagePath(String imageName) {
    return Platform.isWindows
        ? 'assets/$imageName.ico'
        : 'assets/$imageName.png';
  }

  Future<void> initSystemTray() async {
    List<String> iconList = ['darts_icon', 'gift_icon'];

    // We first init the systray menu and then add the menu entries
    await _systemTray.initSystemTray(iconPath: getImagePath('app_icon'));
    _systemTray.setTitle("system tray");
    _systemTray.setToolTip("How to use system tray with Flutter");

    // handle system tray event
    _systemTray.registerSystemTrayEventHandler((eventName) {
      debugPrint("eventName: $eventName");
      if (eventName == kSystemTrayEventClick) {
        Platform.isWindows ? _appWindow.show() : _systemTray.popUpContextMenu();
      } else if (eventName == kSystemTrayEventRightClick) {
        Platform.isWindows ? _systemTray.popUpContextMenu() : _appWindow.show();
      }
    });

    await _menuMain.buildFrom(
      [
        MenuItemLable(
          label: 'Change Context Menu',
          image: 'assets/darts_icon.bmp',
          onClicked: () {
            debugPrint("Change Context Menu");

            _toogleMenu = !_toogleMenu;
            _systemTray.setContextMenu(_toogleMenu ? _menuMain : _menuSimple);
          },
        ),
        MenuSeparator(),
        MenuItemLable(label: 'Show', onClicked: _appWindow.show),
        MenuItemLable(label: 'Hide', onClicked: _appWindow.hide),
        MenuItemLable(
          label: 'Start flash tray icon',
          onClicked: () {
            debugPrint("Start flash tray icon");

            _timer ??= Timer.periodic(
              const Duration(milliseconds: 500),
              (timer) {
                _toogleTrayIcon = !_toogleTrayIcon;
                _systemTray
                    .setImage(_toogleTrayIcon ? "" : getImagePath('app_icon'));
              },
            );
          },
        ),
        MenuItemLable(
          label: 'Stop flash tray icon',
          onClicked: () {
            debugPrint("Stop flash tray icon");

            _timer?.cancel();
            _timer = null;

            _systemTray.setImage(getImagePath('app_icon'));
          },
        ),
        MenuSeparator(),
        SubMenu(
          label: "Test API",
          children: [
            SubMenu(
              label: "setSystemTrayInfo",
              children: [
                MenuItemLable(
                  label: 'setTitle',
                  onClicked: () {
                    final String text = WordPair.random().asPascalCase;
                    debugPrint("click 'setTitle' : $text");
                    _systemTray.setTitle(text);
                  },
                ),
                MenuItemLable(
                  label: 'setImage',
                  onClicked: () {
                    String iconName =
                        iconList[Random().nextInt(iconList.length)];
                    String path = getImagePath(iconName);
                    debugPrint("click 'setImage' : $path");
                    _systemTray.setImage(path);
                  },
                ),
                MenuItemLable(
                  label: 'setToolTip',
                  onClicked: () {
                    final String text = WordPair.random().asPascalCase;
                    debugPrint("click 'setToolTip' : $text");
                    _systemTray.setToolTip(text);
                  },
                ),
                MenuItemLable(
                  label: 'getTitle [macOS]',
                  onClicked: () async {
                    String title = await _systemTray.getTitle();
                    debugPrint("click 'getTitle' : $title");
                  },
                ),
              ],
            ),
            MenuItemLable(label: 'disabled Item', enabled: false),
          ],
        ),
        MenuSeparator(),
        MenuItemCheckbox(
          label: 'Checkbox 1',
          name: 'checkbox1',
          checked: true,
          onClicked: () async {
            debugPrint("click 'Checkbox 1'");

            MenuItemCheckbox? menuItem =
                _menuMain.findItemByName("checkbox1") as MenuItemCheckbox?;
            await menuItem?.setCheck(!menuItem.checked);
            debugPrint(
                "click name: ${menuItem?.name} menuItemId: ${menuItem?.menuItemId} label: ${menuItem?.label} checked: ${menuItem?.checked}");
          },
        ),
        MenuItemCheckbox(
          label: 'Checkbox 2',
          name: 'checkbox2',
          onClicked: () async {
            debugPrint("click 'Checkbox 2'");

            MenuItemCheckbox? menuItem =
                _menuMain.findItemByName("checkbox2") as MenuItemCheckbox?;
            await menuItem?.setCheck(!menuItem.checked);
            await menuItem?.setLable(WordPair.random().asPascalCase);
            debugPrint(
                "click name: ${menuItem?.name} menuItemId: ${menuItem?.menuItemId} label: ${menuItem?.label} checked: ${menuItem?.checked}");
          },
        ),
        MenuItemCheckbox(
          label: 'Checkbox 3',
          name: 'checkbox3',
          checked: true,
          onClicked: () async {
            debugPrint("click 'Checkbox 3'");

            MenuItemCheckbox? menuItem =
                _menuMain.findItemByName("checkbox3") as MenuItemCheckbox?;
            await menuItem?.setCheck(!menuItem.checked);
            debugPrint(
                "click name: ${menuItem?.name} menuItemId: ${menuItem?.menuItemId} label: ${menuItem?.label} checked: ${menuItem?.checked}");
          },
        ),
        MenuSeparator(),
        MenuItemLable(
          label: 'Exit',
          onClicked: _appWindow.close,
        ),
      ],
    );

    await _menuSimple.buildFrom([
      MenuItemLable(
        label: 'Change Context Menu',
        onClicked: () {
          debugPrint("Change Context Menu");

          _toogleMenu = !_toogleMenu;
          _systemTray.setContextMenu(_toogleMenu ? _menuMain : _menuSimple);
        },
      ),
      MenuSeparator(),
      MenuItemLable(label: 'Show', onClicked: _appWindow.show),
      MenuItemLable(label: 'Hide', onClicked: _appWindow.hide),
      MenuItemLable(
        label: 'Exit',
        onClicked: _appWindow.close,
      ),
    ]);

    _systemTray.setContextMenu(_menuMain);
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
        CloseWindowButton(
          colors: closeButtonColors,
          onPressed: () => appWindow.close(),
        ),
      ],
    );
  }
}
