#pragma once

#include "cef_app.h"

class MiniApp final : public CefApp, public CefBrowserProcessHandler {
 public:
  MiniApp() = default;
  ~MiniApp() override = default;

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }

  void OnBeforeCommandLineProcessing(const CefString& process_type,
                                     CefRefPtr<CefCommandLine> command_line) override;

  IMPLEMENT_REFCOUNTING(MiniApp);
};
