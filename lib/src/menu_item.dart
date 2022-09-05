import 'package:flutter/services.dart';
import 'package:system_tray/src/utils.dart';

const String _kSetLabel = "SetLabel";
const String _kSetImage = "SetImage";
const String _kSetEnable = "SetEnable";
const String _kSetCheck = "SetCheck";

const String _kMenuIdKey = 'menu_id';
const String _kMenuItemIdKey = 'menu_item_id';
const String _kIdKey = 'id';
const String _kTypeKey = 'type';
const String _kMenuTypeLabel = 'label';
const String _kMenuTypeCheckbox = 'checkbox';
const String _kMenuTypeSubMenu = 'submenu';
const String _kMenuTypeSeparator = 'separator';
const String _kLabelKey = 'label';
const String _kImageKey = 'image';
const String _kSubMenuKey = 'submenu';
const String _kEnabledKey = 'enabled';
const String _kCheckedKey = 'checked';

/// A callback provided to [MenuItem] to handle menu selection.
typedef MenuItemSelectedCallback = void Function(MenuItem);

/// The base type for an individual menu item that can be shown in a menu.
abstract class MenuItem {
  MenuItem(
    this.type,
    this.label,
    this.image,
    this.name,
    this.enabled,
    this.checked,
    this.onClicked,
  );

  Map<String, dynamic> toJson() {
    return <String, dynamic>{};
  }

  Future<void> setLabel(String label) async {
    bool result = await channel?.invokeMethod(_kSetLabel, {
      _kMenuIdKey: menuId ?? -1,
      _kMenuItemIdKey: menuItemId ?? -1,
      _kLabelKey: label,
    });
    if (result) {
      this.label = label;
    }
  }

  Future<void> setImage(String image) async {
    String? imageAbsolutePath = await Utils.getIcon(image);

    bool result = await channel?.invokeMethod(_kSetImage, {
      _kMenuIdKey: menuId ?? -1,
      _kMenuItemIdKey: menuItemId ?? -1,
      _kImageKey: imageAbsolutePath,
    });
    if (result) {
      this.image = image;
      this.imageAbsolutePath = imageAbsolutePath;
    }
  }

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

  Future<void> setCheck(bool checked) async {
    if (type != _kMenuTypeCheckbox) {
      return;
    }

    bool result = await channel?.invokeMethod(_kSetCheck, {
      _kMenuIdKey: menuId ?? -1,
      _kMenuItemIdKey: menuItemId ?? -1,
      _kCheckedKey: checked,
    });
    if (result) {
      this.checked = checked;
    }
  }

  MethodChannel? channel;
  int? menuId;
  int? menuItemId;
  String? imageAbsolutePath;

  final String type;
  String label;
  String? image;
  String? name;

  /// Whether or not the menu item is enabled.
  bool enabled;

  /// Whether or not the menu item is checked.
  bool checked;

  /// The callback to call whenever the menu item is selected.
  final MenuItemSelectedCallback? onClicked;
}

/// A standard menu item, with no submenus.
class MenuItemLabel extends MenuItem {
  MenuItemLabel({
    required String label,
    String? image,
    String? name,
    bool enabled = true,
    MenuItemSelectedCallback? onClicked,
  }) : super(_kMenuTypeLabel, label, image, name, enabled, false, onClicked);

  @override
  Map<String, dynamic> toJson() {
    return <String, dynamic>{
      _kTypeKey: type,
      _kIdKey: menuItemId,
      _kLabelKey: label,
      _kImageKey: imageAbsolutePath,
      _kEnabledKey: enabled,
    };
  }
}

/// A menu item that serves as a checkbox.
class MenuItemCheckbox extends MenuItem {
  MenuItemCheckbox({
    required String label,
    String? image,
    String? name,
    bool enabled = true,
    bool checked = false,
    MenuItemSelectedCallback? onClicked,
  }) : super(_kMenuTypeCheckbox, label, image, name, enabled, checked,
            onClicked);

  @override
  Map<String, dynamic> toJson() {
    return <String, dynamic>{
      _kTypeKey: type,
      _kIdKey: menuItemId,
      _kLabelKey: label,
      _kImageKey: imageAbsolutePath,
      _kEnabledKey: enabled,
      _kCheckedKey: checked,
    };
  }
}

/// A menu item continaing a submenu.
///
/// The item itself can't be selected, it just displays the submenu.
class SubMenu extends MenuItem {
  /// Creates a new submenu with the given [label] and [children].
  SubMenu({required String label, required this.children, String? image})
      : super(_kMenuTypeSubMenu, label, image, null, true, false, null);

  @override
  Map<String, dynamic> toJson() {
    return <String, dynamic>{
      _kTypeKey: type,
      _kIdKey: menuItemId,
      _kLabelKey: label,
      _kImageKey: imageAbsolutePath,
      _kEnabledKey: enabled,
      _kSubMenuKey: children.map((e) => e.toJson()).toList(),
    };
  }

  /// The menu items contained in the submenu.
  final List<MenuItem> children;
}

/// A menu item that serves as a separator, generally drawn as a line.
class MenuSeparator extends MenuItem {
  /// Creates a new separator item.
  MenuSeparator()
      : super(_kMenuTypeSeparator, '', null, null, true, false, null);

  @override
  Map<String, dynamic> toJson() {
    return <String, dynamic>{
      _kTypeKey: type,
    };
  }
}
