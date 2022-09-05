#include "menu.h"

#include <winuser.h>

#include <memory>

#include "errors.h"
#include "utils.h"

namespace {

constexpr int kDefaultIconSizeWidth = 32;
constexpr int kDefaultIconSizeHeight = 32;

constexpr char kMenuIdKey[] = "menu_id";
constexpr char kMenuItemIdKey[] = "menu_item_id";
constexpr char kMenuListKey[] = "menu_list";
constexpr char kIdKey[] = "id";
constexpr char kTypeKey[] = "type";
constexpr char kSeparatorKey[] = "separator";
constexpr char kSubMenuKey[] = "submenu";
constexpr char kLabelKey[] = "label";
constexpr char kImageKey[] = "image";
constexpr char kEnabledKey[] = "enabled";
constexpr char kCheckedKey[] = "checked";

constexpr char kMenuItemSelectedCallbackMethod[] = "MenuItemSelectedCallback";

}  // namespace

Menu::Menu(std::weak_ptr<flutter::MethodChannel<>> channel,
           int menu_id) noexcept
    : channel_(channel), menu_id_(menu_id) {}

Menu::~Menu() noexcept {
  DestroyContextMenu();
}

bool Menu::CreateContextMenu(
    const flutter::MethodCall<flutter::EncodableValue>& method_call) {
  bool result = false;
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      break;
    }

    const auto* list = std::get_if<flutter::EncodableList>(
        utils::ValueOrNull(*params, kMenuListKey));
    if (!list) {
      break;
    }

    if (!CreateContextMenu(*list)) {
      break;
    }

    result = true;

  } while (false);

  return result;
}

void Menu::SetLabel(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_item_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuItemIdKey));
    if (!menu_item_id) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* label =
        std::get_if<std::string>(utils::ValueOrNull(*params, kLabelKey));
    if (!label) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    SetLabel(*menu_item_id, *label);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void Menu::SetImage(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_item_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuItemIdKey));
    if (!menu_item_id) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* image =
        std::get_if<std::string>(utils::ValueOrNull(*params, kImageKey));
    if (!image) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    SetImage(*menu_item_id, *image);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void Menu::SetEnable(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_item_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuItemIdKey));
    if (!menu_item_id) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* enabled =
        std::get_if<bool>(utils::ValueOrNull(*params, kEnabledKey));
    if (!enabled) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    SetEnable(*menu_item_id, *enabled);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

void Menu::SetCheck(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    const auto* params =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!params) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* menu_item_id =
        std::get_if<int>(utils::ValueOrNull(*params, kMenuItemIdKey));
    if (!menu_item_id) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    const auto* checked =
        std::get_if<bool>(utils::ValueOrNull(*params, kCheckedKey));
    if (!checked) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    SetCheck(*menu_item_id, *checked);

    result.Success(flutter::EncodableValue(true));
    return;

  } while (false);
}

bool Menu::CreateContextMenu(const flutter::EncodableList& representation) {
  bool result = false;

  HMENU menu = nullptr;
  do {
    menu = CreatePopupMenu();
    if (!ValueToMenu(menu, representation)) {
      break;
    }

    menu_ = menu;
    result = true;
  } while (false);

  if (!result) {
    DestroyMenu(menu);
  }

  return result;
}

void Menu::DestroyContextMenu() {
  if (menu_) {
    DestroyMenu(menu_);
    menu_ = nullptr;
  }
}

void Menu::PopupContextMenu(HWND window, const POINT& pt) {
  int menu_item_id =
      static_cast<int>(TrackPopupMenu(GetMenu(), TPM_LEFTBUTTON | TPM_RETURNCMD,
                                      pt.x, pt.y, 0, window, nullptr));
  if (menu_item_id > 0) {
    if (!channel_.expired()) {
      std::shared_ptr<flutter::MethodChannel<>> channel = channel_.lock();
      channel->InvokeMethod(
          kMenuItemSelectedCallbackMethod,
          std::make_unique<flutter::EncodableValue>(
              flutter::EncodableMap{{flutter::EncodableValue(kMenuIdKey),
                                     flutter::EncodableValue(MenuId())},
                                    {flutter::EncodableValue(kMenuItemIdKey),
                                     flutter::EncodableValue(menu_item_id)}}));
    }
  }
}

void Menu::SetLabel(int menu_item_id, const std::string& label) {
  std::wstring label_u = utils::Utf16FromUtf8(label);

  MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
  mii.fMask = MIIM_STRING;
  mii.dwTypeData = label_u.data();
  mii.cch = static_cast<UINT>(label_u.length());
  SetMenuItemInfo(GetMenu(), static_cast<UINT>(menu_item_id), FALSE, &mii);
}

void Menu::SetImage(int menu_item_id, const std::string& image) {
  std::wstring image_u = utils::Utf16FromUtf8(image);

  int x = /*GetSystemMetrics(SM_CXICON)*/ kDefaultIconSizeWidth;
  int y = /*GetSystemMetrics(SM_CYICON)*/ kDefaultIconSizeHeight;

  MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
  mii.fMask = MIIM_BITMAP;
  mii.hbmpItem = static_cast<HBITMAP>(
      LoadImage(nullptr, image_u.c_str(), IMAGE_BITMAP, x, y, LR_LOADFROMFILE));

  SetMenuItemInfo(GetMenu(), static_cast<UINT>(menu_item_id), FALSE, &mii);
}

void Menu::SetEnable(int menu_item_id, bool enabled) {
  MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
  mii.fMask = MIIM_STATE;
  mii.fState = (enabled ? MFS_ENABLED : MFS_DISABLED);
  SetMenuItemInfo(GetMenu(), static_cast<UINT>(menu_item_id), FALSE, &mii);
}

void Menu::SetCheck(int menu_item_id, bool checked) {
  MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
  mii.fMask = MIIM_STATE;
  mii.fState = (checked ? MFS_CHECKED : MFS_UNCHECKED);
  SetMenuItemInfo(GetMenu(), static_cast<UINT>(menu_item_id), FALSE, &mii);
}

int Menu::MenuId() const {
  return menu_id_;
}

HMENU Menu::GetMenu() const {
  return menu_;
}

bool Menu::ValueToMenu(HMENU menu,
                       const flutter::EncodableList& representation) {
  for (const auto& item : representation) {
    if (!ValueToMenuItem(menu, std::get<flutter::EncodableMap>(item))) {
      return false;
    }
  }
  return true;
}

bool Menu::ValueToMenuItem(HMENU menu,
                           const flutter::EncodableMap& representation) {
  const auto* type =
      std::get_if<std::string>(utils::ValueOrNull(representation, kTypeKey));
  if (!type) {
    return false;
  }

  if (type->compare(kSeparatorKey) == 0) {
    AppendMenu(menu, MF_SEPARATOR, 0, nullptr);
  } else {
    const auto* id =
        std::get_if<int32_t>(utils::ValueOrNull(representation, kIdKey));
    UINT menu_item_id = id ? *id : -1;

    const auto* label =
        std::get_if<std::string>(utils::ValueOrNull(representation, kLabelKey));
    std::wstring label_u(label ? utils::Utf16FromUtf8(*label) : L"");

    const auto* image =
        std::get_if<std::string>(utils::ValueOrNull(representation, kImageKey));

    const auto* enabled =
        std::get_if<bool>(utils::ValueOrNull(representation, kEnabledKey));

    MENUITEMINFO mii = {sizeof(MENUITEMINFO)};

    if (id) {
      mii.fMask |= MIIM_ID;
      mii.wID = menu_item_id;
    }

    if (enabled) {
      mii.fMask |= MIIM_STATE;
      mii.fState = *enabled ? MFS_ENABLED : MFS_DISABLED;
    }

    if (label) {
      mii.fMask |= MIIM_STRING;
      mii.dwTypeData = label_u.data();
      mii.cch = static_cast<UINT>(label_u.length());
    }

    if (image) {
      std::wstring image_u = utils::Utf16FromUtf8(*image);

      int x = /*GetSystemMetrics(SM_CXICON)*/ kDefaultIconSizeWidth;
      int y = /*GetSystemMetrics(SM_CYICON)*/ kDefaultIconSizeHeight;

      // printf("cxicon: %d, cyicon: %d, image:%s\n", x, y, image->c_str());

      mii.fMask |= MIIM_BITMAP;
      mii.hbmpItem = static_cast<HBITMAP>(LoadImage(
          nullptr, image_u.c_str(), IMAGE_BITMAP, x, y, LR_LOADFROMFILE));
    }

    mii.fMask |= MIIM_DATA;
    mii.dwItemData = static_cast<ULONG_PTR>(MenuId());

    if (type->compare(kSubMenuKey) == 0) {
      const auto* children = std::get_if<flutter::EncodableList>(
          utils::ValueOrNull(representation, kSubMenuKey));
      if (children) {
        HMENU submenu = ::CreatePopupMenu();
        if (ValueToMenu(submenu, *children)) {
          mii.fMask |= MIIM_SUBMENU;
          mii.hSubMenu = submenu;
          InsertMenuItem(menu, menu_item_id, FALSE, &mii);
        } else {
          DestroyMenu(submenu);
        }
      }
    } else {
      const auto* checked =
          std::get_if<bool>(utils::ValueOrNull(representation, kCheckedKey));
      if (checked) {
        mii.fMask |= MIIM_STATE;
        mii.fState = *checked ? MFS_CHECKED : MFS_UNCHECKED;
      }

      InsertMenuItem(menu, menu_item_id, FALSE, &mii);
    }
  }

  return true;
}
