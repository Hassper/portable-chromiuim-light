#pragma once

#include <string>

#include <windows.h>

#include "browser/client_handler.h"
#include "cef_browser.h"

class BrowserWindow {
 public:
  BrowserWindow(HINSTANCE instance, std::wstring startup_url, bool incognito);
  ~BrowserWindow() = default;

  bool Create();
  void Show(int cmd_show);

  void UpdateTitle(const std::wstring& title);
  void UpdateAddressBar(const std::wstring& url);
  void RefreshTabsUi();

  [[nodiscard]] HWND Handle() const { return hwnd_; }
  [[nodiscard]] CefRefPtr<ClientHandler> Handler() const { return handler_; }

 private:
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
  LRESULT HandleMessage(UINT message, WPARAM wparam, LPARAM lparam);

  void LayoutControls();
  void CreateInitialBrowser();
  void NavigateToAddressBar();
  void ActivateTabByIndex(size_t index);
  void NewTab(const std::wstring& url);

  HINSTANCE instance_ = nullptr;
  std::wstring startup_url_;
  bool incognito_ = false;

  HWND hwnd_ = nullptr;
  HWND toolbar_ = nullptr;
  HWND edit_address_ = nullptr;
  HWND btn_back_ = nullptr;
  HWND btn_forward_ = nullptr;
  HWND btn_reload_ = nullptr;
  HWND tab_list_ = nullptr;

  CefRefPtr<ClientHandler> handler_;
  CefRefPtr<CefBrowser> browser_;
};
