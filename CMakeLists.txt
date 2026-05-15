cmake_minimum_required(VERSION 3.20)

project(
  vocalplayer
  VERSION 0.3.0
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VOCALPLAYER_BUILD_TESTS "Build VocalPlayer tests." ON)
option(VOCALPLAYER_FIND_TAGLIB
  "Search for TagLib (disable for MinGW cross-builds to avoid host Linux includes)."
  ON)

find_path(MINIAUDIO_INCLUDE_DIR miniaudio.h REQUIRED)
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE ${MINIAUDIO_INCLUDE_DIR})

find_package(kissfft CONFIG REQUIRED COMPONENTS float)
find_package(ftxui CONFIG REQUIRED)

if(VOCALPLAYER_FIND_TAGLIB)
  find_package(TagLib CONFIG QUIET)

  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND AND NOT TagLib_FOUND)
    pkg_check_modules(TAGLIB QUIET IMPORTED_TARGET taglib)
  endif()
endif()

if(WIN32 OR MINGW)
  set(VOCALPLAYER_PLATFORM_CONSOLE_SRC src/platform/console_windows.cpp)
  set(VOCALPLAYER_PLATFORM_DECODER_OPEN_SRC
      src/platform/miniaudio_decoder_open_windows.cpp)
else()
  set(VOCALPLAYER_PLATFORM_CONSOLE_SRC src/platform/console_posix.cpp)
  set(VOCALPLAYER_PLATFORM_DECODER_OPEN_SRC
      src/platform/miniaudio_decoder_open_posix.cpp)
endif()

add_library(
  vocalplayer_core
  src/app/app_controller.cpp
  src/app/playlist.cpp
  src/audio/audio_engine.cpp
  src/audio/decoder.cpp
  src/audio/metadata.cpp
  src/analysis/spectrum_analyzer.cpp
  src/ui/tui_renderer.cpp
  src/ui/visual_update_pipeline.cpp
  ${VOCALPLAYER_PLATFORM_CONSOLE_SRC}
  ${VOCALPLAYER_PLATFORM_DECODER_OPEN_SRC})

target_include_directories(
  vocalplayer_core
  PUBLIC
    ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(
  vocalplayer_core
  PUBLIC
    kissfft::kissfft
    miniaudio
    ftxui::screen
    ftxui::dom
    ftxui::component)

set(VOCALPLAYER_TAGLIB_ENABLED OFF)
if(TagLib_FOUND AND TARGET TagLib::tag)
  target_compile_definitions(vocalplayer_core PUBLIC VOCALPLAYER_HAS_TAGLIB=1)
  target_link_libraries(vocalplayer_core PUBLIC TagLib::tag)
  set(VOCALPLAYER_TAGLIB_ENABLED ON)
elseif(TagLib_FOUND AND TARGET TagLib::TagLib)
  target_compile_definitions(vocalplayer_core PUBLIC VOCALPLAYER_HAS_TAGLIB=1)
  target_link_libraries(vocalplayer_core PUBLIC TagLib::TagLib)
  set(VOCALPLAYER_TAGLIB_ENABLED ON)
elseif(TAGLIB_FOUND)
  target_compile_definitions(vocalplayer_core PUBLIC VOCALPLAYER_HAS_TAGLIB=1)
  target_link_libraries(vocalplayer_core PUBLIC PkgConfig::TAGLIB)
  set(VOCALPLAYER_TAGLIB_ENABLED ON)
endif()

if(VOCALPLAYER_TAGLIB_ENABLED)
  message(STATUS "TagLib metadata support: ENABLED")
else()
  message(STATUS "TagLib metadata support: DISABLED (artist/title fallback mode)")
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
      ${PROJECT_SOURCE_DIR}/src)
  target_link_libraries(test_spectrum_analyzer PRIVATE kissfft::kissfft)
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

  add_executable(
    test_coalescing_redraw_gate
    tests/test_coalescing_redraw_gate.cpp)
  target_include_directories(
    test_coalescing_redraw_gate
    PRIVATE
      ${PROJECT_SOURCE_DIR}/src)
  add_test(
    NAME coalescing_redraw_gate_test
    COMMAND test_coalescing_redraw_gate)
endif()
