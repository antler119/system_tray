#ifndef __APPWINDOW_H__
#define __APPWINDOW_H__

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>

extern const char kInitAppWindow[];
extern const char kShowAppWindow[];
extern const char kHideAppWindow[];
extern const char kCloseAppWindow[];

class AppWindow {
 public:
  AppWindow(FlPluginRegistrar* registrar, FlMethodChannel* channel) noexcept;
  ~AppWindow() noexcept;

  void handle_method_call(FlMethodCall* method_call);

 protected:
  FlMethodResponse* init_app_window(FlValue* args);
  FlMethodResponse* show_app_window(FlValue* args);
  FlMethodResponse* hide_app_window(FlValue* args);
  FlMethodResponse* close_app_window(FlValue* args);

  bool init_app_window(GtkWindow* window);
  bool show_app_window();
  bool hide_app_window();
  bool close_app_window();

  static gboolean static_window_state_event_callback_fun(
      GtkWidget* widget,
      GdkEventWindowState* event,
      AppWindow* self);

  gboolean window_state_event_callback_fun(GtkWidget* widget,
                                           GdkEventWindowState* event);

 protected:
  FlPluginRegistrar* registrar_ = nullptr;
  FlMethodChannel* channel_ = nullptr;

  GtkWindow* window_ = nullptr;
  bool window_iconify_ = false;
  gint x_ = -1;
  gint y_ = -1;
};

#endif  // __APPWINDOW_H__
