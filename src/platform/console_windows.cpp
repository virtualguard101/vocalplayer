/**
 * @file console_windows.cpp
 * @brief Windows console UTF-8 initialization for CJK metadata and TUI text.
 */
#include <windows.h>

#include <clocale>
#include <locale>

#include "platform/console.hpp"

namespace vocalplayer {
namespace platform {

void PrepareConsoleEnvironment() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  setlocale(LC_ALL, ".UTF-8");
  try {
    std::locale::global(std::locale(".UTF-8"));
  } catch (...) {  // NOLINT(bugprone-empty-catch)
    // Keep process running even if a UTF-8 locale is not installed.
  }
}

}  // namespace platform
}  // namespace vocalplayer
