import 'package:system_tray/src/platform/system_tray_platform_interface.dart';

const String _kMenuTypeLabel = 'label';
const String _kMenuTypeCheckbox = 'checkbox';
const String _kMenuTypeSubMenu = 'submenu';
const String _kMenuTypeSeparator = 'separator';
const String _kLabelKey = 'label';
const String _kImageKey = 'image';
const String _kSubMenuKey = 'submenu';
const String _kEnabledKey = 'enabled';
const String _kCheckedKey = 'checked';
const String _kIdKey = 'id';
const String _kTypeKey = 'type';

/// A callback provided to [MenuItemBase] to handle menu selection.
typedef MenuItemSelectedCallback = void Function(MenuItemBase);

/// The base type for an individual menu item that can be shown in a menu.
abstract class MenuItemBase {
  MenuItemBase(
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
    if (menuId != null && menuItemId != null) {
      await SystemTrayPlatform.instance.setMenuItemLabel(menuId!, menuItemId!, label);
    }
    this.label = label;
  }

  Future<void> setImage(String image) async {
    if (menuId != null && menuItemId != null) {
      await SystemTrayPlatform.instance.setMenuItemImage(menuId!, menuItemId!, image);
    }
    this.image = image;
  }

  Future<void> setEnable(bool enabled) async {
    if (menuId != null && menuItemId != null) {
      await SystemTrayPlatform.instance.setMenuItemEnable(menuId!, menuItemId!, enabled);
    }
    this.enabled = enabled;
  }

  Future<void> setCheck(bool checked) async {
    if (type != _kMenuTypeCheckbox) {
      return;
    }

    if (menuId != null && menuItemId != null) {
      await SystemTrayPlatform.instance.setMenuItemCheck(menuId!, menuItemId!, checked);
    }
    this.checked = checked;
  }

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
class MenuItemLabel extends MenuItemBase {
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
class MenuItemCheckbox extends MenuItemBase {
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
class SubMenu extends MenuItemBase {
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
  final List<MenuItemBase> children;
}

/// A menu item that serves as a separator, generally drawn as a line.
class MenuSeparator extends MenuItemBase {
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
