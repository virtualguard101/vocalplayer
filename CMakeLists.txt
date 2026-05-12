cmake_minimum_required(VERSION 3.20)

project(
  VocalPlayer
  VERSION 0.2.0
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VOCALPLAYER_BUILD_TESTS "Build VocalPlayer tests." ON)

include(FetchContent)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(KISSFFT_TEST OFF CACHE BOOL "" FORCE)
set(KISSFFT_TOOLS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  miniaudio
  GIT_REPOSITORY https://github.com/mackron/miniaudio.git
  GIT_TAG 0.11.23)
FetchContent_MakeAvailable(miniaudio)

FetchContent_Declare(
  kissfft
  GIT_REPOSITORY https://github.com/mborgerding/kissfft.git
  GIT_TAG 131.1.0)
FetchContent_MakeAvailable(kissfft)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0)
FetchContent_MakeAvailable(ftxui)

add_library(
  kissfft_lib
  STATIC
  ${kissfft_SOURCE_DIR}/kiss_fft.c)
target_include_directories(kissfft_lib PUBLIC ${kissfft_SOURCE_DIR})

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(TAGLIB QUIET IMPORTED_TARGET taglib)
endif()

add_library(
  vocalplayer_core
  src/app/app_controller.cpp
  src/app/playlist.cpp
  src/audio/audio_engine.cpp
  src/audio/decoder.cpp
  src/audio/metadata.cpp
  src/analysis/spectrum_analyzer.cpp
  src/ui/tui_renderer.cpp)

target_include_directories(
  vocalplayer_core
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${miniaudio_SOURCE_DIR}
    ${kissfft_SOURCE_DIR})

target_link_libraries(
  vocalplayer_core
  PUBLIC
    kissfft_lib
    ftxui::screen
    ftxui::dom
    ftxui::component)

if(TAGLIB_FOUND)
  target_compile_definitions(vocalplayer_core PUBLIC VOCALPLAYER_HAS_TAGLIB=1)
  target_link_libraries(vocalplayer_core PUBLIC PkgConfig::TAGLIB)
endif()

add_executable(vocalplayer src/main.cpp)
target_link_libraries(vocalplayer PRIVATE vocalplayer_core)

if(VOCALPLAYER_BUILD_TESTS)
  set(BUILD_TESTING ON CACHE BOOL "" FORCE)
  enable_testing()

  add_executable(
    test_spectrum_analyzer
    tests/test_spectrum_analyzer.cpp
    src/analysis/spectrum_analyzer.cpp)
  target_include_directories(
    test_spectrum_analyzer
    PRIVATE
      ${PROJECT_SOURCE_DIR}/src
      ${kissfft_SOURCE_DIR})
  target_link_libraries(test_spectrum_analyzer PRIVATE kissfft_lib)
  add_test(NAME spectrum_analyzer_test COMMAND test_spectrum_analyzer)

  add_executable(test_playlist tests/test_playlist.cpp src/app/playlist.cpp)
  target_include_directories(test_playlist PRIVATE ${PROJECT_SOURCE_DIR}/src)
  add_test(NAME playlist_test COMMAND test_playlist)

  add_executable(test_keybindings tests/test_keybindings.cpp)
  target_include_directories(test_keybindings PRIVATE ${PROJECT_SOURCE_DIR}/src)
  add_test(NAME keybindings_test COMMAND test_keybindings)

  add_executable(test_theme tests/test_theme.cpp)
  target_include_directories(test_theme PRIVATE ${PROJECT_SOURCE_DIR}/src)
  target_link_libraries(test_theme PRIVATE ftxui::screen)
  add_test(NAME theme_test COMMAND test_theme)
endif()
