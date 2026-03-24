#pragma once

#include <optional>
#include <string>
#include <vector>

#include "cef_browser.h"

class TabManager {
 public:
  struct TabEntry {
    int browser_id;
    CefRefPtr<CefBrowser> browser;
    std::wstring title;
    std::wstring url;
  };

  void Add(CefRefPtr<CefBrowser> browser);
  void Remove(int browser_id);
  void UpdateTitle(int browser_id, const std::wstring& title);
  void UpdateUrl(int browser_id, const std::wstring& url);

  [[nodiscard]] std::optional<TabEntry> GetActive() const;
  [[nodiscard]] std::optional<TabEntry> GetById(int browser_id) const;
  [[nodiscard]] std::vector<TabEntry> GetTabs() const;

  void SetActive(int browser_id);
  [[nodiscard]] int ActiveBrowserId() const { return active_browser_id_; }

 private:
  std::vector<TabEntry> tabs_;
  int active_browser_id_ = -1;
};
