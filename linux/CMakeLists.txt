cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "system_tray")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "${PROJECT_NAME}_plugin")

find_package(PkgConfig REQUIRED)

add_library(${PLUGIN_NAME} SHARED
  "system_tray_plugin.cc"
  "app_window.cc"
  "menu_manager.cc"
  "menu.cc"
  "tray.cc"
  "errors.cc"
)

pkg_check_modules(APPINDICATOR IMPORTED_TARGET ayatana-appindicator3-0.1)
if(APPINDICATOR_FOUND)
  target_compile_definitions(${PLUGIN_NAME} PRIVATE HAVE_AYATANA)
else()
  pkg_check_modules(APPINDICATOR IMPORTED_TARGET appindicator3-0.1)
endif()
if(APPINDICATOR_FOUND)
  target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::APPINDICATOR)
else() 
  message(
    FATAL_ERROR
    "\n"
    "The `tray_manager` package requires ayatana-appindicator3-0.1 or appindicator3-0.1. See https://github.com/leanflutter/tray_manager#linux-requirements"
  )
endif()


apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter)
target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::GTK)
target_link_libraries(${PLUGIN_NAME} PRIVATE PkgConfig::APPINDICATOR)

# List of absolute paths to libraries that should be bundled with the plugin
set(system_tray_bundled_libraries
  ""
  PARENT_SCOPE
)
