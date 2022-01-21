import 'dart:io';
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:system_tray/system_tray.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final SystemTray _systemTray = SystemTray();
  final AppWindow _appWindow = AppWindow();

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

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Flutter Demo',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // Try running your application with "flutter run". You'll see the
        // application has a blue toolbar. Then, without quitting the app, try
        // changing the primarySwatch below to Colors.green and then invoke
        // "hot reload" (press "r" in the console where you ran "flutter run",
        // or simply save your changes to "hot reload" in a Flutter IDE).
        // Notice that the counter didn't reset back to zero; the application
        // is not restarted.
        primarySwatch: Colors.blue,
      ),
      home: Scaffold(
        appBar: AppBar(
          title: const Text('How to use system tray with Flutter'),
        ),
        body: const Center(),
      ),
    );
  }
}
