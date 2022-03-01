#include "menu.h"
#include <winuser.h>

#include <memory>

#include "errors.h"
#include "utils.h"

namespace {

constexpr char kChannelName[] = "flutter/system_tray/menu_manager";

constexpr char kMenuIdKey[] = "menu_id";
constexpr char kMenuItemIdKey[] = "menu_item_id";
constexpr char kIdKey[] = "id";
constexpr char kTypeKey[] = "type";
constexpr char kCheckboxKey[] = "checkbox";
constexpr char kSeparatorKey[] = "separator";
constexpr char kSubMenuKey[] = "submenu";
constexpr char kLabelKey[] = "label";
constexpr char kImageKey[] = "image";
constexpr char kEnabledKey[] = "enabled";
constexpr char kCheckedKey[] = "checked";

constexpr char kMenuItemSelectedCallbackMethod[] = "MenuItemSelectedCallback";

}  // namespace

Menu::Menu(std::weak_ptr<flutter::MethodChannel<>> channel, int menu_id)
    : channel_(channel), menu_id_(menu_id) {}

Menu::~Menu() noexcept {
  printf("~Menu this: %p\n", this);
  DestroyContextMenu();
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

void Menu::SetLable(int menu_item_id, const std::string& label) {
  printf("SetLable menu_item_id:%d text:%s\n", menu_item_id, label.c_str());

  std::wstring label_u = utils::Utf16FromUtf8(label);

  MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
  mii.fMask = MIIM_STRING;
  mii.dwTypeData = label_u.data();
  mii.cch = static_cast<UINT>(label_u.length());
  SetMenuItemInfo(GetMenu(), static_cast<UINT>(menu_item_id), FALSE, &mii);
}

void Menu::SetImage(int menu_item_id, const std::string& image) {}

void Menu::SetEnable(int menu_item_id, bool enabled) {
  MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
  mii.fMask = MIIM_STATE;
  mii.fState = (enabled ? MFS_ENABLED : MFS_GRAYED);
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
    const auto* label =
        std::get_if<std::string>(utils::ValueOrNull(representation, kLabelKey));
    std::wstring label_u(label ? utils::Utf16FromUtf8(*label) : L"");

    const auto* image =
        std::get_if<std::string>(utils::ValueOrNull(representation, kImageKey));

    const auto* enabled =
        std::get_if<bool>(utils::ValueOrNull(representation, kEnabledKey));

    if (type->compare(kSubMenuKey) == 0) {
      UINT flags = MF_STRING | MF_POPUP;
      flags |= (enabled == nullptr || *enabled) ? MF_ENABLED : MF_GRAYED;

      const auto* children = std::get_if<flutter::EncodableList>(
          utils::ValueOrNull(representation, kSubMenuKey));
      if (children) {
        HMENU submenu = ::CreatePopupMenu();
        if (ValueToMenu(submenu, *children)) {
          AppendMenu(menu, flags, reinterpret_cast<UINT_PTR>(submenu),
                     label_u.c_str());
        } else {
          DestroyMenu(submenu);
        }
      }
    } else {
      const auto* menu_id =
          std::get_if<int32_t>(utils::ValueOrNull(representation, kIdKey));
      UINT item_id = menu_id ? *menu_id : 0;

      MENUITEMINFO mii = {sizeof(MENUITEMINFO)};
      mii.fMask = MIIM_ID | MIIM_STRING | MIIM_DATA;
      mii.wID = item_id;
      mii.dwTypeData = label_u.data();
      mii.cch = static_cast<UINT>(label_u.length());
      mii.dwItemData = static_cast<ULONG_PTR>(MenuId());

      const auto* checked =
          std::get_if<bool>(utils::ValueOrNull(representation, kCheckedKey));
      if (checked) {
        mii.fMask |= MIIM_STATE;
        mii.fState = *checked ? MFS_CHECKED : MFS_UNCHECKED;
      }

      if (image) {
        std::wstring image_u = utils::Utf16FromUtf8(*image);

        int x = GetSystemMetrics(SM_CXICON);
        int y = GetSystemMetrics(SM_CYICON);

        printf("cxicon: %d, cyicon: %d\n", x, y);

        mii.fMask |= MIIM_BITMAP;
        mii.hbmpItem = static_cast<HBITMAP>(LoadImage(
            nullptr, image_u.c_str(), IMAGE_BITMAP, 32, 32, LR_LOADFROMFILE));
      }

      InsertMenuItem(menu, item_id, FALSE, &mii);
    }
  }

  return true;
}
