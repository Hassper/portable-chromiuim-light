#include "browser/client_handler.h"

#include <filesystem>
#include <sstream>

#include "cef_parser.h"
#include "cef_task.h"
#include "ui/browser_window.h"

ClientHandler::ClientHandler(BrowserWindow* owner_window) : owner_window_(owner_window) {}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
  tabs_.UpdateTitle(browser->GetIdentifier(), title.ToWString());
  if (owner_window_) {
    owner_window_->UpdateTitle(title.ToWString());
    owner_window_->RefreshTabsUi();
  }
}

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    const CefString& url) {
  if (!frame->IsMain()) {
    return;
  }

  tabs_.UpdateUrl(browser->GetIdentifier(), url.ToWString());
  if (owner_window_) {
    owner_window_->UpdateAddressBar(url.ToWString());
    owner_window_->RefreshTabsUi();
  }
}

bool ClientHandler::OnBeforePopup(CefRefPtr<CefBrowser>,
                                  CefRefPtr<CefFrame>,
                                  const CefString&,
                                  const CefString&,
                                  CefLifeSpanHandler::WindowOpenDisposition,
                                  bool,
                                  const CefPopupFeatures&,
                                  CefWindowInfo&,
                                  CefRefPtr<CefClient>&,
                                  CefBrowserSettings&,
                                  CefRefPtr<CefDictionaryValue>&,
                                  bool*) {
  // Block popups for a strict lightweight policy.
  return true;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  tabs_.Add(browser);
  tabs_.SetActive(browser->GetIdentifier());
  if (owner_window_) {
    owner_window_->RefreshTabsUi();
  }
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser>) {
  return false;
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  tabs_.Remove(browser->GetIdentifier());
  if (owner_window_) {
    owner_window_->RefreshTabsUi();
  }
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode error_code,
                                const CefString& error_text,
                                const CefString& failed_url) {
  if (error_code == ERR_ABORTED || !frame->IsMain()) {
    return;
  }

  std::stringstream html;
  html << "<html><body><h2>Load error</h2><p>URL: " << failed_url.ToString() << "</p><p>Error: "
       << error_text.ToString() << " (" << error_code << ")</p></body></html>";

  frame->LoadString(html.str(), failed_url);
}

void ClientHandler::OnBeforeDownload(CefRefPtr<CefBrowser>,
                                     CefRefPtr<CefDownloadItem>,
                                     const CefString& suggested_name,
                                     CefRefPtr<CefBeforeDownloadCallback> callback) {
  if (!callback) {
    return;
  }

  const auto download_path = std::filesystem::temp_directory_path() / suggested_name.ToString();
  callback->Continue(download_path.string(), true);
}

void ClientHandler::OnDownloadUpdated(CefRefPtr<CefBrowser>,
                                      CefRefPtr<CefDownloadItem> download_item,
                                      CefRefPtr<CefDownloadItemCallback>) {
  if (!download_item) {
    return;
  }

  // Hook for progress integration into status UI in future.
}
