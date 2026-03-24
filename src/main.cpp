#include <windows.h>

#include <string>

#include "app/mini_app.h"
#include "cef_command_line.h"
#include "cef_sandbox_win.h"
#include "ui/browser_window.h"

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int cmd_show) {
  CefEnableHighDPISupport();

  void* sandbox_info = nullptr;
#if defined(CEF_USE_SANDBOX)
  sandbox_info = CefSandboxInfoCreate();
#endif

  CefMainArgs main_args(instance);
  CefRefPtr<MiniApp> app(new MiniApp());

  const int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0) {
    return exit_code;
  }

  CefSettings settings;
  settings.no_sandbox = false;
  settings.command_line_args_disabled = false;
  settings.persist_session_cookies = false;
  settings.persist_user_preferences = false;
  settings.multi_threaded_message_loop = false;

  CefString(&settings.cache_path) = L"";
  CefString(&settings.root_cache_path) = L"";
  CefString(&settings.log_file) = L"cef.log";
  settings.log_severity = LOGSEVERITY_DISABLE;

  if (!CefInitialize(main_args, settings, app, sandbox_info)) {
    return -1;
  }

  auto command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromString(::GetCommandLineW());

  const bool incognito = command_line->HasSwitch("incognito");
  const std::wstring startup_url = command_line->HasSwitch("url")
                                       ? command_line->GetSwitchValue(L"url").ToWString()
                                       : L"https://example.com";

  BrowserWindow window(instance, startup_url, incognito);
  if (!window.Create()) {
    CefShutdown();
    return -2;
  }

  window.Show(cmd_show);

  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
    CefDoMessageLoopWork();
  }

  CefShutdown();

#if defined(CEF_USE_SANDBOX)
  CefSandboxInfoDestroy(sandbox_info);
#endif

  return 0;
}
