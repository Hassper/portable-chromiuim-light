#include "browser/tab_manager.h"

#include <algorithm>

void TabManager::Add(CefRefPtr<CefBrowser> browser) {
  if (!browser) {
    return;
  }

  const int id = browser->GetIdentifier();
  tabs_.push_back(TabEntry{id, browser, L"New Tab", browser->GetMainFrame()->GetURL().ToWString()});
  active_browser_id_ = id;
}

void TabManager::Remove(int browser_id) {
  const auto end_it = std::remove_if(tabs_.begin(), tabs_.end(), [browser_id](const TabEntry& entry) {
    return entry.browser_id == browser_id;
  });

  const bool removed_active = active_browser_id_ == browser_id;
  tabs_.erase(end_it, tabs_.end());

  if (tabs_.empty()) {
    active_browser_id_ = -1;
  } else if (removed_active) {
    active_browser_id_ = tabs_.front().browser_id;
  }
}

void TabManager::UpdateTitle(int browser_id, const std::wstring& title) {
  for (auto& tab : tabs_) {
    if (tab.browser_id == browser_id) {
      tab.title = title;
      return;
    }
  }
}

void TabManager::UpdateUrl(int browser_id, const std::wstring& url) {
  for (auto& tab : tabs_) {
    if (tab.browser_id == browser_id) {
      tab.url = url;
      return;
    }
  }
}

std::optional<TabManager::TabEntry> TabManager::GetActive() const {
  return GetById(active_browser_id_);
}

std::optional<TabManager::TabEntry> TabManager::GetById(int browser_id) const {
  for (const auto& tab : tabs_) {
    if (tab.browser_id == browser_id) {
      return tab;
    }
  }
  return std::nullopt;
}

std::vector<TabManager::TabEntry> TabManager::GetTabs() const {
  return tabs_;
}

void TabManager::SetActive(int browser_id) {
  if (GetById(browser_id).has_value()) {
    active_browser_id_ = browser_id;
  }
}
