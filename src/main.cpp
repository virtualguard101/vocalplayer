/**
 * @file main.cpp
 * @brief Defines the CLI entry point for VocalPlayer.
 *
 * Key points:
 * - Applies `platform::PrepareConsoleEnvironment()` before UI startup.
 * - Validates required command-line input path.
 * - Constructs AppController and delegates runtime orchestration.
 * - Returns process-style exit code for shell integration.
 */
#include <iostream>
#include <string>

#include "app/app_controller.hpp"
#include "platform/console.hpp"

// CLI entry point. Delegates runtime orchestration to AppController.
int main(int argc, char** argv) {
  vocalplayer::platform::PrepareConsoleEnvironment();

  if (argc < 2) {
    std::cerr << "Usage: vocalplayer <audio-file-or-directory>\n";
    return 1;
  }

  const std::string input_path = argv[1];
  vocalplayer::AppController app;
  return app.Run(input_path);
}
