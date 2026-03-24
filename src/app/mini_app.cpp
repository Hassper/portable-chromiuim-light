#include "app/mini_app.h"

namespace {

void AppendDisable(CefRefPtr<CefCommandLine> cmd, const char* name) {
  if (!cmd->HasSwitch(name)) {
    cmd->AppendSwitch(name);
  }
}

void AppendDisableWithValue(CefRefPtr<CefCommandLine> cmd, const char* name, const char* value) {
  if (!cmd->HasSwitch(name)) {
    cmd->AppendSwitchWithValue(name, value);
  }
}

}  // namespace

void MiniApp::OnBeforeCommandLineProcessing(const CefString& process_type,
                                             CefRefPtr<CefCommandLine> command_line) {
  // Global performance and privacy defaults.
  AppendDisable(command_line, "disable-background-networking");
  AppendDisable(command_line, "disable-background-timer-throttling");
  AppendDisable(command_line, "disable-breakpad");
  AppendDisable(command_line, "disable-component-update");
  AppendDisable(command_line, "disable-default-apps");
  AppendDisable(command_line, "disable-domain-reliability");
  AppendDisable(command_line, "disable-hang-monitor");
  AppendDisable(command_line, "disable-ipc-flooding-protection");
  AppendDisable(command_line, "disable-popup-blocking");
  AppendDisable(command_line, "disable-prompt-on-repost");
  AppendDisable(command_line, "disable-renderer-backgrounding");
  AppendDisable(command_line, "disable-sync");
  AppendDisable(command_line, "metrics-recording-only");
  AppendDisable(command_line, "no-default-browser-check");
  AppendDisable(command_line, "no-first-run");

  AppendDisableWithValue(command_line, "autoplay-policy", "no-user-gesture-required");
  AppendDisableWithValue(command_line, "disable-features",
                         "MediaRouter,OptimizationHints,Translate,AutofillServerCommunication,"
                         "CertificateTransparencyComponentUpdater,InterestFeedContentSuggestions");

  if (!command_line->HasSwitch("disable-gpu")) {
    command_line->AppendSwitch("enable-gpu");
  }
}
