#include "app_window.h"

#include "errors.h"

namespace {

constexpr char kChannelName[] = "flutter/system_tray/app_window";

constexpr char kInitAppWindow[] = "InitAppWindow";
constexpr char kShowAppWindow[] = "ShowAppWindow";
constexpr char kHideAppWindow[] = "HideAppWindow";
constexpr char kCloseAppWindow[] = "CloseAppWindow";

}  // namespace

AppWindow::AppWindow(flutter::PluginRegistrarWindows* registrar) noexcept
    : registrar_(registrar) {
  assert(registrar_);

  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar_->messenger(), kChannelName,
          &flutter::StandardMethodCodec::GetInstance());

  channel->SetMethodCallHandler([this](const auto& call, auto result) {
    HandleMethodCall(call, std::move(result));
  });

  channel_ = std::move(channel);
}

AppWindow::~AppWindow() noexcept {
  registrar_ = nullptr;
}

void AppWindow::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  // printf("method call %s\n", method_call.method_name().c_str());

  if (method_call.method_name().compare(kInitAppWindow) == 0) {
    initAppWindow(method_call, *result);
  } else if (method_call.method_name().compare(kShowAppWindow) == 0) {
    showAppWindow(method_call, *result);
  } else if (method_call.method_name().compare(kHideAppWindow) == 0) {
    hideAppWindow(method_call, *result);
  } else if (method_call.method_name().compare(kCloseAppWindow) == 0) {
    closeAppWindow(method_call, *result);
  } else {
    result->NotImplemented();
  }
}

void AppWindow::initAppWindow(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    flutter::FlutterView* view = registrar_->GetView();
    if (!view) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    HWND flutter_window = view->GetNativeWindow();
    HWND window = GetAncestor(flutter_window, GA_ROOT);

    if (!initAppWindow(window, flutter_window)) {
      result.Error(errors::kBadArgumentsError, "",
                   flutter::EncodableValue(false));
      break;
    }

    result.Success(flutter::EncodableValue(true));

  } while (false);
}

void AppWindow::showAppWindow(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    showAppWindow(true);

    result.Success(flutter::EncodableValue(true));
  } while (false);
}

void AppWindow::hideAppWindow(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    showAppWindow(false);

    result.Success(flutter::EncodableValue(true));
  } while (false);
}

void AppWindow::closeAppWindow(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    flutter::MethodResult<flutter::EncodableValue>& result) {
  do {
    closeAppWindow();

    result.Success(flutter::EncodableValue(true));
  } while (false);
}

bool AppWindow::initAppWindow(HWND window, HWND flutter_window) {
  window_ = window;
  flutter_window_ = flutter_window;
  return true;
}

bool AppWindow::showAppWindow(bool visible) {
  if (!IsWindow(window_)) {
    return false;
  }

  if (visible) {
    activeWindow();
    refreshFlutterWindow();
  } else {
    ShowWindow(window_, SW_HIDE);
  }
  return true;
}

bool AppWindow::closeAppWindow() {
  if (!IsWindow(window_)) {
    return false;
  }

  PostMessage(window_, WM_SYSCOMMAND, SC_CLOSE, 0);
  return true;
}

void AppWindow::activeWindow() {
  if (!IsWindow(window_)) {
    return;
  }

  if (!::IsWindowVisible(window_)) {
    ShowWindow(window_, SW_SHOW);
  }

  if (IsIconic(window_)) {
    SendMessage(window_, WM_SYSCOMMAND, SC_RESTORE | HTCAPTION, 0);
  }

  BringWindowToTop(window_);
  SetForegroundWindow(window_);
}

void AppWindow::refreshFlutterWindow() {
  if (!IsWindow(flutter_window_)) {
    return;
  }

  RECT rc = {};
  GetClientRect(flutter_window_, &rc);
  int width = rc.right - rc.left;
  int height = rc.bottom - rc.top;
  SetWindowPos(flutter_window_, 0, 0, 0, width + 1, height + 1,
               SWP_NOMOVE | SWP_NOACTIVATE);
  SetWindowPos(flutter_window_, 0, 0, 0, width, height,
               SWP_NOMOVE | SWP_NOACTIVATE);
}
