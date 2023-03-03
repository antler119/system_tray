#ifndef __TRAY_H__
#define __TRAY_H__

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#ifdef HAVE_AYATANA
#include <libayatana-appindicator/app-indicator.h>
#else
#include <libappindicator/app-indicator.h>
#endif
#include <memory>

typedef AppIndicator* (*app_indicator_new_fun)(const gchar*,
                                               const gchar*,
                                               AppIndicatorCategory);

typedef void (*app_indicator_set_status_fun)(AppIndicator*, AppIndicatorStatus);
typedef void (*app_indicator_set_icon_full_func)(AppIndicator* self,
                                                 const gchar* icon_name,
                                                 const gchar* icon_desc);
typedef void (*app_indicator_set_attention_icon_full_fun)(AppIndicator*,
                                                          const gchar*,
                                                          const gchar*);
typedef void (*app_indicator_set_label_func)(AppIndicator* self,
                                             const gchar* label,
                                             const gchar* guide);

typedef void (*app_indicator_set_title_func)(AppIndicator* self,
                                             const gchar* title);

typedef const gchar* (*app_indicator_get_label_func)(AppIndicator* self);

typedef void (*app_indicator_set_menu_fun)(AppIndicator*, GtkMenu*);

extern const char kInitSystemTray[];
extern const char kSetSystemTrayInfo[];
extern const char kSetContextMenu[];
extern const char kPopupContextMenu[];
extern const char kGetTitle[];
extern const char kDestroySystemTray[];

class MenuManager;

class Tray {
 public:
  Tray(FlMethodChannel* _channel,
       std::weak_ptr<MenuManager> menu_manager) noexcept;
  ~Tray() noexcept;

  void handle_method_call(FlMethodCall* method_call);

 protected:
  FlMethodResponse* init_tray(FlValue* args);
  FlMethodResponse* set_tray_info(FlValue* args);
  FlMethodResponse* set_context_menu(FlValue* args);
  FlMethodResponse* popup_context_menu(FlValue* args);
  FlMethodResponse* get_title(FlValue* args);
  FlMethodResponse* destroy_system_tray(FlValue* args);

  bool init_tray(const char* tray_id);
  bool set_tray_info(const char* title,
                     const char* icon_path,
                     const char* toolTip);
  void set_context_menu(int64_t context_menu_id);
  int64_t get_context_menu_id() const;

  bool init_indicator_api();
  bool create_indicator(const char* tray_id);
  void destroy_indicator();
  void hide_indicator();

 protected:
  app_indicator_new_fun app_indicator_new_ = nullptr;
  app_indicator_set_status_fun app_indicator_set_status_ = nullptr;
  app_indicator_set_icon_full_func app_indicator_set_icon_full_ = nullptr;
  app_indicator_set_attention_icon_full_fun
      app_indicator_set_attention_icon_full_ = nullptr;
  app_indicator_set_label_func app_indicator_set_label_ = nullptr;
  app_indicator_set_title_func app_indicator_set_title_ = nullptr;
  app_indicator_get_label_func app_indicator_get_label_ = nullptr;
  app_indicator_set_menu_fun app_indicator_set_menu_ = nullptr;

  FlMethodChannel* channel_ = nullptr;
  std::weak_ptr<MenuManager> menu_manager_;

  bool indicator_api_inited_ = false;

  AppIndicator* app_indicator_ = nullptr;

  int context_menu_id_ = -1;
};

#endif  // __TRAY_H__