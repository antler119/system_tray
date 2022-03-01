import 'package:flutter/services.dart';
import 'package:system_tray/src/utils.dart';

const String _kSetLable = "setLable";
const String _kSetImage = "setImage";
const String _kSetEnable = "setEnable";
const String _kSetCheck = "setCheck";

const String _kMenuIdKey = 'menu_id';
const String _kMenuItemIdKey = 'menu_item_id';
const String _kIdKey = 'id';
const String _kTypeKey = 'type';
const String _kLabelKey = 'label';
const String _kImageKey = 'image';
const String _kSubMenuKey = 'submenu';
const String _kEnabledKey = 'enabled';
const String _kCheckedKey = 'checked';

/// A callback provided to [MenuItem] to handle menu selection.
typedef MenuItemSelectedCallback = void Function();

/// The base type for an individual menu item that can be shown in a menu.
abstract class MenuItem {
  /// Creates a new menu item with the give label.
  MenuItem(this.type, this.label, this.image, this.name);

  Future<Map<String, dynamic>> toJson() async => <String, dynamic>{
        _kTypeKey: type,
        _kLabelKey: label,
        _kImageKey: Utils.getIcon(image),
        _kIdKey: menuItemId,
      };

  Future<void> setLable(String label) async {
    bool result = await channel?.invokeMethod(_kSetLable, {
      _kMenuIdKey: menuId ?? -1,
      _kMenuItemIdKey: menuItemId ?? -1,
      _kLabelKey: label,
    });
    if (result) {
      this.label = label;
    }
  }

  Future<void> setImage(String image) async {
    bool result = await channel?.invokeMethod(_kSetImage, {
      _kMenuIdKey: menuId ?? -1,
      _kMenuItemIdKey: menuItemId ?? -1,
      _kImageKey: image,
    });
    if (result) {
      this.image = image;
    }
  }

  Future<void> setEnable(bool enabled) async {}
  Future<void> setCheck(bool checked) async {}

  MethodChannel? channel;

  /// The displayed label for the menu item.
  final String type;
  String label;
  String? image;
  String? name;
  int? menuId;
  int? menuItemId;
}

/// A standard menu item, with no submenus.
abstract class _MenuItemWithCallback extends MenuItem {
  /// Creates a new menu item with the given [label] and options.
  ///
  /// Note that onClicked should generally be set unless [enabled] is false,
  /// or the menu item will be selectable but not do anything.
  _MenuItemWithCallback(
    String type,
    String label,
    String? image,
    String? name,
    this.enabled,
    this.onClicked,
  ) : super(type, label, image, name);

  @override
  Future<Map<String, dynamic>> toJson() async => <String, dynamic>{
        _kTypeKey: type,
        _kLabelKey: label,
        _kImageKey: Utils.getIcon(image),
        _kIdKey: menuItemId,
        _kEnabledKey: enabled,
      };

  @override
  Future<void> setEnable(bool enabled) async {
    bool result = await channel?.invokeMethod(_kSetEnable, {
      _kMenuIdKey: menuId ?? -1,
      _kMenuItemIdKey: menuItemId ?? -1,
      _kEnabledKey: enabled,
    });
    if (result) {
      this.enabled = enabled;
    }
  }

  /// Whether or not the menu item is enabled.
  bool enabled;

  /// The callback to call whenever the menu item is selected.
  final MenuItemSelectedCallback? onClicked;
}

/// A standard menu item, with no submenus.
class MenuItemLable extends _MenuItemWithCallback {
  /// Creates a new menu item with the given [label] and options.
  ///
  /// Note that onClicked should generally be set unless [enabled] is false,
  /// or the menu item will be selectable but not do anything.
  MenuItemLable({
    required String label,
    String? image,
    String? name,
    bool enabled = true,
    MenuItemSelectedCallback? onClicked,
  }) : super('lable', label, image, name, enabled, onClicked);
}

/// A menu item that serves as a checkbox.
class MenuItemCheckbox extends _MenuItemWithCallback {
  /// Creates a new menu checkbox with the given [label] and options.
  ///
  /// Note that onClicked should generally be set unless [enabled] is false,
  /// or the menu checkbox will be selectable but not do anything.
  MenuItemCheckbox({
    required String label,
    String? image,
    String? name,
    bool enabled = true,
    this.checked = false,
    MenuItemSelectedCallback? onClicked,
  }) : super('checkbox', label, image, name, enabled, onClicked);

  @override
  Future<Map<String, dynamic>> toJson() async => <String, dynamic>{
        _kTypeKey: type,
        _kLabelKey: label,
        _kImageKey: Utils.getIcon(image),
        _kIdKey: menuItemId,
        _kEnabledKey: enabled,
        _kCheckedKey: checked
      };

  @override
  Future<void> setCheck(bool checked) async {
    bool result = await channel?.invokeMethod(_kSetCheck, {
      _kMenuIdKey: menuId ?? -1,
      _kMenuItemIdKey: menuItemId ?? -1,
      _kCheckedKey: checked,
    });
    if (result) {
      this.checked = checked;
    }
  }

  bool checked;
}

/// A menu item continaing a submenu.
///
/// The item itself can't be selected, it just displays the submenu.
class SubMenu extends MenuItem {
  /// Creates a new submenu with the given [label] and [children].
  SubMenu({required String label, required this.children, String? image})
      : super('submenu', label, image, null);

  @override
  Future<Map<String, dynamic>> toJson() async => <String, dynamic>{
        _kTypeKey: type,
        _kLabelKey: label,
        _kImageKey: Utils.getIcon(image),
        _kIdKey: menuItemId,
        _kSubMenuKey: children.map((e) async => await e.toJson()).toList(),
      };

  /// The menu items contained in the submenu.
  final List<MenuItem> children;
}

/// A menu item that serves as a separator, generally drawn as a line.
class MenuSeparator extends MenuItem {
  /// Creates a new separator item.
  MenuSeparator() : super('separator', '', null, null);
}
