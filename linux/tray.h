#ifndef __Tray_H__
#define __Tray_H__

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

typedef AppIndicator* (*app_indicator_new_fun)(const gchar*,
                                               const gchar*,
                                               AppIndicatorCategory);
typedef void (*app_indicator_set_status_fun)(AppIndicator*, AppIndicatorStatus);
typedef void (*app_indicator_set_attention_icon_full_fun)(AppIndicator*,
                                                          const gchar*,
                                                          const gchar*);
typedef void (*app_indicator_set_menu_fun)(AppIndicator*, GtkMenu*);

class SystemTray {
 public:
  bool init_system_tray(const char* title,
                        const char* iconPath,
                        const char* toolTip);
  bool set_context_menu(GtkWidget* system_menu);

 protected:
  bool init_indicator_api();
  bool create_indicator(const char* title,
                        const char* iconPath,
                        const char* toolTip);

 protected:
  app_indicator_new_fun _app_indicator_new = nullptr;
  app_indicator_set_status_fun _app_indicator_set_status = nullptr;
  app_indicator_set_attention_icon_full_fun
      _app_indicator_set_attention_icon_full = nullptr;
  app_indicator_set_menu_fun _app_indicator_set_menu = nullptr;

  AppIndicator* _app_indicator = nullptr;
};

#endif  // __Tray_H__