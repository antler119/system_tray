#ifndef __MENU_H__
#define __MENU_H__

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <memory>

class Menu {
 public:
  Menu(FlMethodChannel* channel, int menu_id) noexcept;
  ~Menu() noexcept;

  bool create_context_menu(FlValue* args);
  FlMethodResponse* set_label(FlValue* args);
  FlMethodResponse* set_image(FlValue* args);
  FlMethodResponse* set_enable(FlValue* args);
  FlMethodResponse* set_check(FlValue* args);

  GtkWidget* get_menu() const;

 protected:
  GtkWidget* value_to_menu(int64_t menu_id, FlValue* value);
  GtkWidget* value_to_menu_item(int64_t menu_id, FlValue* value);

  static void menu_item_callback(GtkMenuItem* item, gpointer user_data);
  void handle_menu_item_callback(GtkMenuItem* item, gpointer user_data);

  int64_t menu_id() const;

  void set_label(int64_t menu_item_id, const char* label);
  void set_image(int64_t menu_item_id, const char* image);
  void set_enable(int64_t menu_item_id, bool enabled);
  void set_check(int64_t menu_item_id, bool checked);

 protected:
  FlMethodChannel* channel_ = nullptr;

  int64_t menu_id_ = -1;

  GtkWidget* gtk_menu_ = nullptr;
};

#endif  // __MENU_H__