#include <iostream>
#include <string>

#include "app/app_controller.hpp"

// CLI entry point. Delegates runtime orchestration to AppController.
int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: vocalplayer <audio-file-or-directory>" << std::endl;
    return 1;
  }

  const std::string input_path = argv[1];
  vocalplayer::AppController app;
  return app.Run(input_path);
}
