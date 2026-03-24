# Portable Chromium Light (CEF)

Минималистичный браузер на базе Chromium Embedded Framework (CEF), ориентированный на портативность и низкое потребление ресурсов.

## Возможности

- Портативный запуск из одной папки (без инсталлятора и без записи в реестр).
- Минимальный UI:
  - Назад / Вперёд / Обновить
  - Адресная строка
  - Базовый список вкладок (tabs)
- Простой загрузчик файлов (сохранение в `%TEMP%` по умолчанию).
- Поддержка флагов запуска:
  - `--incognito`
  - `--disable-gpu`
  - `--url=https://...`
- Включён sandbox (`no_sandbox = false`).
- Аппаратное ускорение включено по умолчанию (если не передан `--disable-gpu`).

## Структура

- `src/main.cpp` — инициализация CEF, разбор параметров запуска, цикл сообщений.
- `src/app/mini_app.*` — глобальные Chromium/CEF флаги для минимизации фона и телеметрии.
- `src/browser/client_handler.*` — обработчики вкладок, загрузок, ошибок страниц.
- `src/browser/tab_manager.*` — простой менеджер вкладок.
- `src/ui/browser_window.*` — минимальный Win32 UI и интеграция окна браузера.

## Сборка (Windows, Release)

### 1) Подготовка

1. Установите Visual Studio 2022 (Desktop development with C++).
2. Установите CMake 3.20+.
3. Скачайте CEF Binary Distribution (Windows x64) с поддержкой `libcef_dll_wrapper`.
4. Распакуйте CEF в:

```text
third_party/cef
```

Ожидаются каталоги:

- `third_party/cef/include`
- `third_party/cef/Release`
- `third_party/cef/libcef_dll`

### 2) Конфигурация и сборка

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### 3) Портативный запуск

Скопируйте в одну папку:

- `portable_chromium_light.exe`
- `libcef.dll`
- `icudtl.dat`
- `*.pak`
- `locales\*`
- `chrome_elf.dll` (если присутствует в вашей сборке CEF)
- `swiftshader\*` (опционально)
- `vk_swiftshader\*` (опционально)

Запуск:

```powershell
portable_chromium_light.exe --incognito --url=https://example.com
```

## Отключённые Chromium-компоненты / фичи

Через `OnBeforeCommandLineProcessing`:

- Background networking (`disable-background-networking`)
- Background timer throttling (`disable-background-timer-throttling`)
- Breakpad / crash reporting (`disable-breakpad`)
- Component update (`disable-component-update`)
- Default apps (`disable-default-apps`)
- Domain reliability (`disable-domain-reliability`)
- Hang monitor (`disable-hang-monitor`)
- Renderer backgrounding (`disable-renderer-backgrounding`)
- Sync (`disable-sync`)
- Metrics (`metrics-recording-only`)
- First-run/default-browser checks (`no-first-run`, `no-default-browser-check`)
- Feature-set:
  - MediaRouter
  - OptimizationHints
  - Translate
  - AutofillServerCommunication
  - CertificateTransparencyComponentUpdater
  - InterestFeedContentSuggestions

В коде CEF settings:

- `persist_session_cookies = false`
- `persist_user_preferences = false`
- `cache_path = ""`

## Производительность и размер

В `CMakeLists.txt` включены:

- Release-оптимизации (`/O2`)
- Линк-оптимизации (`/GL`, `/LTCG`)
- Удаление неиспользуемого (`/OPT:REF`, `/OPT:ICF`)
- Статический runtime для уменьшения внешних зависимостей (`MSVC_RUNTIME_LIBRARY=MultiThreaded`)

## Рекомендации по дальнейшей оптимизации

1. **Single-process режим не использовать** (нестабилен), но можно сократить число renderer-процессов ограничением Site Isolation (с осторожностью для безопасности).
2. Отключить дополнительные media-кодеки/подсистемы через сборку Chromium/CEF с кастомным GN-конфигом.
3. Убрать неиспользуемые ресурсы `.pak` только после теста локалей и UI-элементов CEF.
4. Добавить lazy-инициализацию вкладок (создавать рендер только при активации).
5. Вынести загрузки в отдельную настраиваемую portable-папку (`./downloads`).
6. Добавить ограничение RAM для кэшей рендера через дополнительные Chromium switches.
