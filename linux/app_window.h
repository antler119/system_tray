#ifndef __AppWindow_H__
#define __AppWindow_H__

#include <gtk/gtk.h>

class AppWindow {
 public:
  bool init_app_window(GtkWindow* window);
  bool show_app_window();
  bool hide_app_window();
  bool close_app_window();

 protected:
  static gboolean static_window_state_event_callback_fun(
      GtkWidget* widget,
      GdkEventWindowState* event,
      AppWindow* self);

  gboolean window_state_event_callback_fun(GtkWidget* widget,
                                           GdkEventWindowState* event);

 protected:
  GtkWindow* window_ = nullptr;
  bool window_iconify_ = false;
  gint x_ = -1;
  gint y_ = -1;
};

#endif  // __AppWindow_H__
