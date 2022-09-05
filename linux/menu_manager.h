#ifndef __MENU_MANAGER_H__
#define __MENU_MANAGER_H__

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <memory>
#include <unordered_map>

extern const char kCreateContextMenu[];
extern const char kSetLabel[];
extern const char kSetImage[];
extern const char kSetEnable[];
extern const char kSetCheck[];

class Menu;

class MenuManager {
 public:
  MenuManager(FlMethodChannel* channel) noexcept;
  ~MenuManager() noexcept;

  void handle_method_call(FlMethodCall* method_call);

  std::shared_ptr<Menu> get_menu(int64_t menu_id);

 protected:
  FlMethodResponse* create_context_menu(FlValue* args);
  FlMethodResponse* set_label(FlValue* args);
  FlMethodResponse* set_image(FlValue* args);
  FlMethodResponse* set_enable(FlValue* args);
  FlMethodResponse* set_check(FlValue* args);

 protected:
  bool add_menu(int64_t menu_id, std::unique_ptr<Menu> menu);
  std::shared_ptr<Menu> get_menu(FlValue* args);

 protected:
  FlMethodChannel* channel_ = nullptr;

  std::unordered_map<int64_t, std::shared_ptr<Menu>> menus_map_;
};

#endif  // __MENU_MANAGER_H__