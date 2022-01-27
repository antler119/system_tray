#include "app_window.h"

#include <gdk/gdk.h>

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