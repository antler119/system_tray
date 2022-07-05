#include "app_window.h"

#include <gdk/gdk.h>
#include "errors.h"

constexpr char kInitAppWindow[] = "InitAppWindow";
constexpr char kShowAppWindow[] = "ShowAppWindow";
constexpr char kHideAppWindow[] = "HideAppWindow";
constexpr char kCloseAppWindow[] = "CloseAppWindow";

// static
gboolean AppWindow::static_window_state_event_callback_fun(
    GtkWidget* widget,
    GdkEventWindowState* event,
    AppWindow* self) {
  return self->window_state_event_callback_fun(widget, event);
}

gboolean AppWindow::window_state_event_callback_fun(
    GtkWidget* widget,
    GdkEventWindowState* event) {
  if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED) {
    window_iconify_ = true;
  } else {
    window_iconify_ = false;
  }
  return TRUE;
}

AppWindow::AppWindow(FlPluginRegistrar* registrar,
                     FlMethodChannel* channel) noexcept
    : registrar_(registrar), channel_(channel) {}

AppWindow::~AppWindow() noexcept {
  channel_ = nullptr;
}

void AppWindow::handle_method_call(FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  // g_print("method call %s\n", method);

  if (strcmp(method, kInitAppWindow) == 0) {
    response = init_app_window(args);
  } else if (strcmp(method, kShowAppWindow) == 0) {
    response = show_app_window(args);
  } else if (strcmp(method, kHideAppWindow) == 0) {
    response = hide_app_window(args);
  } else if (strcmp(method, kCloseAppWindow) == 0) {
    response = close_app_window(args);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error)) {
    g_warning("Failed to send method call response: %s", error->message);
  }
}

FlMethodResponse* AppWindow::init_app_window(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    FlView* view = fl_plugin_registrar_get_view(registrar_);
    if (view == nullptr) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", fl_value_new_bool(FALSE)));
      break;
    }

    GtkWindow* window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(view)));
    if (window == nullptr) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", fl_value_new_bool(FALSE)));
      break;
    }

    if (!init_app_window(window)) {
      response = FL_METHOD_RESPONSE(fl_method_error_response_new(
          errors::kBadArgumentsError, "", fl_value_new_bool(FALSE)));
      break;
    }

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

FlMethodResponse* AppWindow::show_app_window(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (!show_app_window()) {
      break;
    }

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

FlMethodResponse* AppWindow::hide_app_window(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (!hide_app_window()) {
      break;
    }

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

FlMethodResponse* AppWindow::close_app_window(FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_bool(FALSE);
  FlMethodResponse* response = nullptr;

  do {
    if (!close_app_window()) {
      break;
    }

    result = fl_value_new_bool(TRUE);

  } while (false);

  if (nullptr == response) {
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  }
  return response;
}

bool AppWindow::init_app_window(GtkWindow* window) {
  window_ = window;
  g_signal_connect(
      G_OBJECT(window_), "window-state-event",
      G_CALLBACK(AppWindow::static_window_state_event_callback_fun), this);
  return true;
}

bool AppWindow::show_app_window() {
  if (!window_) {
    return false;
  }

  if (x_ != -1 && y_ != -1) {
    gtk_window_move(window_, x_, y_);
    x_ = -1;
    y_ = -1;
  }

  gtk_widget_show(GTK_WIDGET(window_));
  gtk_window_present(window_);

  if (window_iconify_) {
    gtk_window_deiconify(window_);
  }

  // GdkWindow* gdk_window = gtk_widget_get_window(GTK_WIDGET(window_));
  // GdkDisplay* display = gdk_window_get_display(gdk_window);
  // gdk_display_flush(display);
  return true;
}

bool AppWindow::hide_app_window() {
  if (!window_) {
    return false;
  }

  gtk_window_get_position(window_, &x_, &y_);
  gtk_widget_hide(GTK_WIDGET(window_));
  return true;
}

bool AppWindow::close_app_window() {
  if (!window_) {
    return false;
  }

  gtk_window_close(window_);
  return true;
}