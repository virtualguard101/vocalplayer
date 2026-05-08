#ifndef VOCALPLAYER_SRC_APP_APP_CONTROLLER_HPP_
#define VOCALPLAYER_SRC_APP_APP_CONTROLLER_HPP_

#include <string>
#include <vector>

#include "analysis/spectrum_analyzer.hpp"
#include "app/playlist.hpp"
#include "audio/audio_engine.hpp"
#include "audio/decoder.hpp"
#include "audio/metadata.hpp"
#include "ui/tui_renderer.hpp"

namespace vocalplayer {

class AppController {
 public:
  int Run(const std::string& input_path);

 private:
  bool PlaySingleTrack(const std::string& track_path, int current_index,
                       int total_tracks);

  Decoder decoder_;
  MetadataReader metadata_reader_;
  AudioEngine audio_engine_;
  SpectrumAnalyzer analyzer_{2048, 48, 0.85f};
  TuiRenderer tui_renderer_;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_APP_APP_CONTROLLER_HPP_
