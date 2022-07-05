#include "include/system_tray/system_tray_plugin.h"

#include <map>
#include <memory>

// This must be included before many other Windows headers.
#include <windows.h>
// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/plugin_registrar_windows.h>

#include "app_window.h"
#include "menu_manager.h"
#include "tray.h"
#include "utils.h"

namespace {

class SystemTrayPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

  SystemTrayPlugin(flutter::PluginRegistrarWindows* registrar);

  virtual ~SystemTrayPlugin();

 protected:
  // The registrar for this plugin.
  flutter::PluginRegistrarWindows* registrar_ = nullptr;

  std::unique_ptr<AppWindow> app_window_;
  std::shared_ptr<MenuManager> menu_manager_;
  std::unique_ptr<Tray> tray_;
};

SystemTrayPlugin::SystemTrayPlugin(flutter::PluginRegistrarWindows* registrar)
    : registrar_(registrar) {
  app_window_ = std::make_unique<AppWindow>(registrar_);
  menu_manager_ = std::make_shared<MenuManager>(registrar_);
  tray_ = std::make_unique<Tray>(registrar_, menu_manager_);
}

SystemTrayPlugin::~SystemTrayPlugin() {}

// static
void SystemTrayPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows* registrar) {
  auto plugin = std::make_unique<SystemTrayPlugin>(registrar);
  registrar->AddPlugin(std::move(plugin));
}

}  // namespace

void SystemTrayPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  SystemTrayPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
