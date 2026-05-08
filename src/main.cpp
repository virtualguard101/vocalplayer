#include <iostream>
#include <string>

#include "app/app_controller.hpp"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: vocalplayer <audio-file>" << std::endl;
    return 1;
  }

  const std::string audio_file_path = argv[1];
  vocalplayer::AppController app;
  return app.Run(audio_file_path);
}
