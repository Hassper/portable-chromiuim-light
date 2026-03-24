#include "ui/browser_window.h"

#include <algorithm>
#include <string>

#include "cef_command_line.h"
#include "cef_sandbox_win.h"

namespace {
constexpr wchar_t kWindowClassName[] = L"PortableChromiumLightWindow";
constexpr int kControlHeight = 28;
constexpr int kTabWidth = 220;

enum ControlId : int {
  kIdBack = 1001,
  kIdForward = 1002,
  kIdReload = 1003,
  kIdAddress = 1004,
  kIdTabList = 1005,
};
}  // namespace

BrowserWindow::BrowserWindow(HINSTANCE instance, std::wstring startup_url, bool incognito)
    : instance_(instance), startup_url_(std::move(startup_url)), incognito_(incognito) {}

bool BrowserWindow::Create() {
  WNDCLASSW wc{};
  wc.lpfnWndProc = BrowserWindow::WndProc;
  wc.hInstance = instance_;
  wc.lpszClassName = kWindowClassName;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

  RegisterClassW(&wc);

  hwnd_ = CreateWindowExW(0, kWindowClassName, L"Portable Chromium Light", WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, nullptr, nullptr, instance_, this);

  if (!hwnd_) {
    return false;
  }

  btn_back_ = CreateWindowW(L"BUTTON", L"<", WS_CHILD | WS_VISIBLE, 8, 8, 32, kControlHeight, hwnd_,
                            reinterpret_cast<HMENU>(kIdBack), instance_, nullptr);
  btn_forward_ = CreateWindowW(L"BUTTON", L">", WS_CHILD | WS_VISIBLE, 44, 8, 32, kControlHeight, hwnd_,
                               reinterpret_cast<HMENU>(kIdForward), instance_, nullptr);
  btn_reload_ = CreateWindowW(L"BUTTON", L"R", WS_CHILD | WS_VISIBLE, 80, 8, 32, kControlHeight, hwnd_,
                              reinterpret_cast<HMENU>(kIdReload), instance_, nullptr);
  edit_address_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", startup_url_.c_str(),
                                  WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 116, 8, 760, kControlHeight,
                                  hwnd_, reinterpret_cast<HMENU>(kIdAddress), instance_, nullptr);

  tab_list_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
                              WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL, 8, 42, 240, 700, hwnd_,
                              reinterpret_cast<HMENU>(kIdTabList), instance_, nullptr);

  handler_ = new ClientHandler(this);
  CreateInitialBrowser();

  return true;
}

void BrowserWindow::Show(int cmd_show) {
  ShowWindow(hwnd_, cmd_show);
  UpdateWindow(hwnd_);
}

void BrowserWindow::UpdateTitle(const std::wstring& title) {
  std::wstring window_title = title.empty() ? L"Portable Chromium Light" : title;
  SetWindowTextW(hwnd_, window_title.c_str());
}

void BrowserWindow::UpdateAddressBar(const std::wstring& url) {
  SetWindowTextW(edit_address_, url.c_str());
}

void BrowserWindow::RefreshTabsUi() {
  SendMessageW(tab_list_, LB_RESETCONTENT, 0, 0);
  const auto tabs = handler_->Tabs().GetTabs();
  const int active_id = handler_->Tabs().ActiveBrowserId();

  for (size_t i = 0; i < tabs.size(); ++i) {
    const auto& tab = tabs[i];
    const std::wstring title = tab.title.empty() ? tab.url : tab.title;
    const int idx = static_cast<int>(SendMessageW(tab_list_, LB_ADDSTRING, 0,
                                                  reinterpret_cast<LPARAM>(title.c_str())));
    if (tab.browser_id == active_id) {
      SendMessageW(tab_list_, LB_SETCURSEL, idx, 0);
    }
  }
}

LRESULT CALLBACK BrowserWindow::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  auto* self = reinterpret_cast<BrowserWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

  if (message == WM_NCCREATE) {
    auto* cs = reinterpret_cast<CREATESTRUCTW*>(lparam);
    self = reinterpret_cast<BrowserWindow*>(cs->lpCreateParams);
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    self->hwnd_ = hwnd;
  }

  if (self) {
    return self->HandleMessage(message, wparam, lparam);
  }

  return DefWindowProcW(hwnd, message, wparam, lparam);
}

LRESULT BrowserWindow::HandleMessage(UINT message, WPARAM wparam, LPARAM lparam) {
  switch (message) {
    case WM_SIZE:
      LayoutControls();
      return 0;
    case WM_COMMAND: {
      const int control_id = LOWORD(wparam);
      const int notify_code = HIWORD(wparam);

      if (control_id == kIdBack && browser_) {
        browser_->GoBack();
      } else if (control_id == kIdForward && browser_) {
        browser_->GoForward();
      } else if (control_id == kIdReload && browser_) {
        browser_->Reload();
      } else if (control_id == kIdAddress && notify_code == EN_UPDATE) {
        // noop
      } else if (control_id == kIdAddress && notify_code == EN_MAXTEXT) {
        NavigateToAddressBar();
      } else if (control_id == kIdTabList && notify_code == LBN_SELCHANGE) {
        const int index = static_cast<int>(SendMessageW(tab_list_, LB_GETCURSEL, 0, 0));
        if (index >= 0) {
          ActivateTabByIndex(static_cast<size_t>(index));
        }
      }

      if (control_id == kIdAddress && notify_code == 0) {
        NavigateToAddressBar();
      }
      return 0;
    }
    case WM_DESTROY:
      if (browser_) {
        browser_->GetHost()->CloseBrowser(true);
        browser_ = nullptr;
      }
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProcW(hwnd_, message, wparam, lparam);
  }
}

void BrowserWindow::LayoutControls() {
  RECT rc{};
  GetClientRect(hwnd_, &rc);

  const int width = rc.right - rc.left;
  const int height = rc.bottom - rc.top;

  MoveWindow(btn_back_, 8, 8, 32, kControlHeight, TRUE);
  MoveWindow(btn_forward_, 44, 8, 32, kControlHeight, TRUE);
  MoveWindow(btn_reload_, 80, 8, 32, kControlHeight, TRUE);
  MoveWindow(edit_address_, 116, 8, std::max(200, width - 124), kControlHeight, TRUE);
  MoveWindow(tab_list_, 8, 42, kTabWidth, std::max(120, height - 50), TRUE);

  if (browser_) {
    CefRect browser_rect(kTabWidth + 16, 42, std::max(200, width - kTabWidth - 24),
                         std::max(120, height - 50));
    browser_->GetHost()->WasResized();
    SetWindowPos(browser_->GetHost()->GetWindowHandle(), nullptr, browser_rect.x, browser_rect.y,
                 browser_rect.width, browser_rect.height, SWP_NOZORDER);
  }
}

void BrowserWindow::CreateInitialBrowser() {
  CefWindowInfo window_info;
  RECT rc{};
  GetClientRect(hwnd_, &rc);
  window_info.SetAsChild(hwnd_, CefRect(kTabWidth + 16, 42, rc.right - kTabWidth - 24, rc.bottom - 50));

  CefBrowserSettings settings;
  settings.background_color = CefColorSetARGB(255, 255, 255, 255);

  CefRequestContextSettings context_settings;
  context_settings.persist_session_cookies = 0;
  context_settings.persist_user_preferences = 0;
  if (incognito_) {
    context_settings.cache_path = "";
  }

  CefRefPtr<CefRequestContext> context = CefRequestContext::CreateContext(context_settings, nullptr);
  browser_ = CefBrowserHost::CreateBrowserSync(window_info, handler_, startup_url_, settings, context, nullptr);
}

void BrowserWindow::NavigateToAddressBar() {
  wchar_t buffer[2048] = {0};
  GetWindowTextW(edit_address_, buffer, static_cast<int>(std::size(buffer)));

  std::wstring url(buffer);
  if (url.empty()) {
    return;
  }

  if (url.find(L"://") == std::wstring::npos) {
    url = L"https://" + url;
  }

  if (browser_) {
    browser_->GetMainFrame()->LoadURL(url);
  }
}

void BrowserWindow::ActivateTabByIndex(size_t index) {
  const auto tabs = handler_->Tabs().GetTabs();
  if (index >= tabs.size()) {
    return;
  }

  handler_->Tabs().SetActive(tabs[index].browser_id);
  browser_ = tabs[index].browser;
  UpdateAddressBar(tabs[index].url);
  UpdateTitle(tabs[index].title);
  LayoutControls();
}

void BrowserWindow::NewTab(const std::wstring& url) {
  CefWindowInfo window_info;
  RECT rc{};
  GetClientRect(hwnd_, &rc);
  window_info.SetAsChild(hwnd_, CefRect(kTabWidth + 16, 42, rc.right - kTabWidth - 24, rc.bottom - 50));

  CefBrowserSettings settings;
  browser_ = CefBrowserHost::CreateBrowserSync(window_info, handler_, url, settings, nullptr, nullptr);
}
