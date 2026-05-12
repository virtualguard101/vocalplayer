/**
 * @file main.cpp
 * @brief Defines the CLI entry point for VocalPlayer.
 *
 * Key points:
 * - Validates required command-line input path.
 * - Constructs AppController and delegates runtime orchestration.
 * - Returns process-style exit code for shell integration.
 */
#include <iostream>
#include <string>

#if defined(_WIN32)
#include <windows.h>

#include <clocale>
#include <locale>
#endif

#include "app/app_controller.hpp"

#if defined(_WIN32)
// Enable UTF-8 console IO so CJK metadata can render correctly.
void InitializeWindowsUtf8Console() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  setlocale(LC_ALL, ".UTF-8");
  try {
    std::locale::global(std::locale(".UTF-8"));
  } catch (...) {
    // Keep process running even if a UTF-8 locale is not installed.
  }
}
#endif

// CLI entry point. Delegates runtime orchestration to AppController.
int main(int argc, char** argv) {
#if defined(_WIN32)
  InitializeWindowsUtf8Console();
#endif

  if (argc < 2) {
    std::cerr << "Usage: vocalplayer <audio-file-or-directory>\n";
    return 1;
  }

  const std::string input_path = argv[1];
  vocalplayer::AppController app;
  return app.Run(input_path);
}
