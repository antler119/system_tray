/// A callback provided to [MenuItem] to handle menu selection.
typedef MenuItemSelectedCallback = void Function();

/// The base type for an individual menu item that can be shown in a menu.
abstract class MenuItem {
  /// Creates a new menu item with the give label.
  MenuItem(this.type, this.label);

  /// The displayed label for the menu item.
  final String type;
  String label;
}

/// A standard menu item, with no submenus.
class MenuItemWithCallback extends MenuItem {
  /// Creates a new menu item with the given [label] and options.
  ///
  /// Note that onClicked should generally be set unless [enabled] is false,
  /// or the menu item will be selectable but not do anything.
  MenuItemWithCallback(
    String type,
    String label,
    this.enabled,
    this.onClicked,
  ) : super(type, label);

  /// Whether or not the menu item is enabled.
  bool enabled;

  /// The callback to call whenever the menu item is selected.
  final MenuItemSelectedCallback? onClicked;
}

/// A standard menu item, with no submenus.
class MenuItemLable extends MenuItemWithCallback {
  /// Creates a new menu item with the given [label] and options.
  ///
  /// Note that onClicked should generally be set unless [enabled] is false,
  /// or the menu item will be selectable but not do anything.
  MenuItemLable({
    required String label,
    bool enabled = true,
    MenuItemSelectedCallback? onClicked,
  }) : super('lable', label, enabled, onClicked);
}

/// A menu item that serves as a checkbox.
class MenuItemCheckbox extends MenuItemWithCallback {
  /// Creates a new menu checkbox with the given [label] and options.
  ///
  /// Note that onClicked should generally be set unless [enabled] is false,
  /// or the menu checkbox will be selectable but not do anything.
  MenuItemCheckbox({
    required String label,
    bool enabled = true,
    this.checked = false,
    MenuItemSelectedCallback? onClicked,
  }) : super('checkbox', label, enabled, onClicked);

  bool checked;
}

/// A menu item continaing a submenu.
///
/// The item itself can't be selected, it just displays the submenu.
class SubMenu extends MenuItem {
  /// Creates a new submenu with the given [label] and [children].
  SubMenu({required String label, required this.children})
      : super('submenu', label);

  /// The menu items contained in the submenu.
  final List<MenuItem> children;
}

/// A menu item that serves as a separator, generally drawn as a line.
class MenuSeparator extends MenuItem {
  /// Creates a new separator item.
  MenuSeparator() : super('separator', '');
}
