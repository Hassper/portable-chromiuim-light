cmake_minimum_required(VERSION 3.20)
project(PortableChromiumLight LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /permissive- /O2 /GL /DNOMINMAX /DWIN32_LEAN_AND_MEAN)
  add_link_options(/LTCG /OPT:REF /OPT:ICF)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -O3 -ffunction-sections -fdata-sections)
  add_link_options(-Wl,--gc-sections)
endif()

set(CEF_ROOT "${CMAKE_SOURCE_DIR}/third_party/cef" CACHE PATH "Path to CEF binary distribution root")
set(CEF_INCLUDE_DIR "${CEF_ROOT}/include")
set(CEF_LIB_DIR "${CEF_ROOT}/Release")
set(CEF_WRAPPER_DIR "${CEF_ROOT}/libcef_dll")

if(NOT EXISTS "${CEF_INCLUDE_DIR}/cef_app.h")
  message(FATAL_ERROR "CEF headers not found. Place CEF binary distribution in third_party/cef")
endif()

add_subdirectory(${CEF_WRAPPER_DIR} libcef_dll_wrapper)

add_executable(portable_chromium_light WIN32
  src/main.cpp
  src/app/mini_app.cpp
  src/browser/client_handler.cpp
  src/browser/tab_manager.cpp
  src/ui/browser_window.cpp
)

target_include_directories(portable_chromium_light PRIVATE
  ${CEF_INCLUDE_DIR}
  src
)

target_link_directories(portable_chromium_light PRIVATE ${CEF_LIB_DIR})
target_link_libraries(portable_chromium_light PRIVATE
  libcef
  libcef_dll_wrapper
  shlwapi
  user32
  gdi32
)

target_compile_definitions(portable_chromium_light PRIVATE
  UNICODE
  _UNICODE
)

if(MSVC)
  set_target_properties(portable_chromium_light PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
