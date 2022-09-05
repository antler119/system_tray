import 'dart:async';
import 'dart:io';
import 'dart:math';

import 'package:bitsdojo_window/bitsdojo_window.dart';
import 'package:english_words/english_words.dart';
import 'package:flutter/material.dart' hide MenuItem;
import 'package:system_tray/system_tray.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(
    const MyApp(),
  );

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

String getTrayImagePath(String imageName) {
  return Platform.isWindows ? 'assets/$imageName.ico' : 'assets/$imageName.png';
}

String getImagePath(String imageName) {
  return Platform.isWindows ? 'assets/$imageName.bmp' : 'assets/$imageName.png';
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

  Future<void> initSystemTray() async {
    List<String> iconList = ['darts_icon', 'gift_icon'];

    // We first init the systray menu and then add the menu entries
    await _systemTray.initSystemTray(iconPath: getTrayImagePath('app_icon'));
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
        MenuItemLabel(
          label: 'Change Context Menu',
          image: getImagePath('darts_icon'),
          onClicked: (menuItem) {
            debugPrint("Change Context Menu");

            _toogleMenu = !_toogleMenu;
            _systemTray.setContextMenu(_toogleMenu ? _menuMain : _menuSimple);
          },
        ),
        MenuSeparator(),
        MenuItemLabel(
            label: 'Show',
            image: getImagePath('darts_icon'),
            onClicked: (menuItem) => _appWindow.show()),
        MenuItemLabel(
            label: 'Hide',
            image: getImagePath('darts_icon'),
            onClicked: (menuItem) => _appWindow.hide()),
        MenuItemLabel(
          label: 'Start flash tray icon',
          image: getImagePath('darts_icon'),
          onClicked: (menuItem) {
            debugPrint("Start flash tray icon");

            _timer ??= Timer.periodic(
              const Duration(milliseconds: 500),
              (timer) {
                _toogleTrayIcon = !_toogleTrayIcon;
                _systemTray.setImage(
                    _toogleTrayIcon ? "" : getTrayImagePath('app_icon'));
              },
            );
          },
        ),
        MenuItemLabel(
          label: 'Stop flash tray icon',
          image: getImagePath('darts_icon'),
          onClicked: (menuItem) {
            debugPrint("Stop flash tray icon");

            _timer?.cancel();
            _timer = null;

            _systemTray.setImage(getTrayImagePath('app_icon'));
          },
        ),
        MenuSeparator(),
        SubMenu(
          label: "Test API",
          image: getImagePath('gift_icon'),
          children: [
            SubMenu(
              label: "setSystemTrayInfo",
              image: getImagePath('darts_icon'),
              children: [
                MenuItemLabel(
                  label: 'setTitle',
                  image: getImagePath('darts_icon'),
                  onClicked: (menuItem) {
                    final String text = WordPair.random().asPascalCase;
                    debugPrint("click 'setTitle' : $text");
                    _systemTray.setTitle(text);
                  },
                ),
                MenuItemLabel(
                  label: 'setImage',
                  image: getImagePath('gift_icon'),
                  onClicked: (menuItem) {
                    String iconName =
                        iconList[Random().nextInt(iconList.length)];
                    String path = getTrayImagePath(iconName);
                    debugPrint("click 'setImage' : $path");
                    _systemTray.setImage(path);
                  },
                ),
                MenuItemLabel(
                  label: 'setToolTip',
                  image: getImagePath('darts_icon'),
                  onClicked: (menuItem) {
                    final String text = WordPair.random().asPascalCase;
                    debugPrint("click 'setToolTip' : $text");
                    _systemTray.setToolTip(text);
                  },
                ),
                MenuItemLabel(
                  label: 'getTitle',
                  image: getImagePath('gift_icon'),
                  onClicked: (menuItem) async {
                    String title = await _systemTray.getTitle();
                    debugPrint("click 'getTitle' : $title");
                  },
                ),
              ],
            ),
            MenuItemLabel(
                label: 'disabled Item',
                name: 'disableItem',
                image: getImagePath('gift_icon'),
                enabled: false),
          ],
        ),
        MenuSeparator(),
        MenuItemLabel(
          label: 'Set Item Image',
          onClicked: (menuItem) async {
            debugPrint("click 'SetItemImage'");

            String iconName = iconList[Random().nextInt(iconList.length)];
            String path = getImagePath(iconName);

            await menuItem.setImage(path);
            debugPrint(
                "click name: ${menuItem.name} menuItemId: ${menuItem.menuItemId} label: ${menuItem.label} image: ${menuItem.image}");
          },
        ),
        MenuItemCheckbox(
          label: 'Checkbox 1',
          name: 'checkbox1',
          checked: true,
          onClicked: (menuItem) async {
            debugPrint("click 'Checkbox 1'");

            MenuItemCheckbox? checkbox1 =
                _menuMain.findItemByName<MenuItemCheckbox>("checkbox1");
            await checkbox1?.setCheck(!checkbox1.checked);

            MenuItemCheckbox? checkbox2 =
                _menuMain.findItemByName<MenuItemCheckbox>("checkbox2");
            await checkbox2?.setEnable(checkbox1?.checked ?? true);

            debugPrint(
                "click name: ${checkbox1?.name} menuItemId: ${checkbox1?.menuItemId} label: ${checkbox1?.label} checked: ${checkbox1?.checked}");
          },
        ),
        MenuItemCheckbox(
          label: 'Checkbox 2',
          name: 'checkbox2',
          onClicked: (menuItem) async {
            debugPrint("click 'Checkbox 2'");

            await menuItem.setCheck(!menuItem.checked);
            await menuItem.setLabel(WordPair.random().asPascalCase);
            debugPrint(
                "click name: ${menuItem.name} menuItemId: ${menuItem.menuItemId} label: ${menuItem.label} checked: ${menuItem.checked}");
          },
        ),
        MenuItemCheckbox(
          label: 'Checkbox 3',
          name: 'checkbox3',
          checked: true,
          onClicked: (menuItem) async {
            debugPrint("click 'Checkbox 3'");

            await menuItem.setCheck(!menuItem.checked);
            debugPrint(
                "click name: ${menuItem.name} menuItemId: ${menuItem.menuItemId} label: ${menuItem.label} checked: ${menuItem.checked}");
          },
        ),
        MenuSeparator(),
        MenuItemLabel(
            label: 'Exit', onClicked: (menuItem) => _appWindow.close()),
      ],
    );

    await _menuSimple.buildFrom([
      MenuItemLabel(
        label: 'Change Context Menu',
        image: getImagePath('app_icon'),
        onClicked: (menuItem) {
          debugPrint("Change Context Menu");

          _toogleMenu = !_toogleMenu;
          _systemTray.setContextMenu(_toogleMenu ? _menuMain : _menuSimple);
        },
      ),
      MenuSeparator(),
      MenuItemLabel(
          label: 'Show',
          image: getImagePath('app_icon'),
          onClicked: (menuItem) => _appWindow.show()),
      MenuItemLabel(
          label: 'Hide',
          image: getImagePath('app_icon'),
          onClicked: (menuItem) => _appWindow.hide()),
      MenuItemLabel(
        label: 'Exit',
        image: getImagePath('app_icon'),
        onClicked: (menuItem) => _appWindow.close(),
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
          child: Column(
            children: [
              const TitleBar(),
              ContentBody(
                systemTray: _systemTray,
                menu: _menuMain,
              ),
            ],
          ),
        ),
      ),
    );
  }
}

const backgroundStartColor = Color(0xFFFFD500);
const backgroundEndColor = Color(0xFFF6A00C);

class TitleBar extends StatelessWidget {
  const TitleBar({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return WindowTitleBarBox(
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
    );
  }
}

class ContentBody extends StatelessWidget {
  final SystemTray systemTray;
  final Menu menu;

  const ContentBody({
    Key? key,
    required this.systemTray,
    required this.menu,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: Container(
        color: const Color(0xFFFFFFFF),
        child: ListView(
          padding: const EdgeInsets.symmetric(vertical: 4.0),
          children: [
            Card(
              elevation: 2.0,
              margin: const EdgeInsets.symmetric(
                horizontal: 16.0,
                vertical: 8.0,
              ),
              child: Padding(
                padding: const EdgeInsets.all(12.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'systemTray.initSystemTray',
                      style: TextStyle(
                        fontWeight: FontWeight.w600,
                      ),
                    ),
                    const Text(
                      'Create system tray.',
                    ),
                    const SizedBox(
                      height: 12.0,
                    ),
                    ElevatedButton(
                      child: const Text("initSystemTray"),
                      onPressed: () async {
                        if (await systemTray.initSystemTray(
                            iconPath: getTrayImagePath('app_icon'))) {
                          systemTray.setTitle("new system tray");
                          systemTray.setToolTip(
                              "How to use system tray with Flutter");
                          systemTray.setContextMenu(menu);
                        }
                      },
                    ),
                  ],
                ),
              ),
            ),
            Card(
              elevation: 2.0,
              margin: const EdgeInsets.symmetric(
                horizontal: 16.0,
                vertical: 8.0,
              ),
              child: Padding(
                padding: const EdgeInsets.all(12.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text(
                      'systemTray.destroy',
                      style: TextStyle(
                        fontWeight: FontWeight.w600,
                      ),
                    ),
                    const Text(
                      'Destroy system tray.',
                    ),
                    const SizedBox(
                      height: 12.0,
                    ),
                    ElevatedButton(
                      child: const Text("destroy"),
                      onPressed: () async {
                        await systemTray.destroy();
                      },
                    ),
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
          onPressed: () {
            showDialog<void>(
              context: context,
              barrierDismissible: false,
              builder: (BuildContext context) {
                return AlertDialog(
                  title: const Text('Exit Program?'),
                  content: const Text(
                      ('The window will be hidden, to exit the program you can use the system menu.')),
                  actions: <Widget>[
                    TextButton(
                      child: const Text('OK'),
                      onPressed: () {
                        Navigator.of(context).pop();
                        appWindow.hide();
                      },
                    ),
                  ],
                );
              },
            );
          },
        ),
      ],
    );
  }
}
