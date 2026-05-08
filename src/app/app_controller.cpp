#include "app/app_controller.hpp"

#include <exception>
#include <iostream>

namespace vocalplayer {

int AppController::Run(const std::string& audio_file_path) {
  try {
    DecodedTrack decoded = decoder_.DecodeFile(audio_file_path);
    TrackInfo track_info =
        metadata_reader_.ReadTrackInfo(audio_file_path, decoded.sample_rate_hz,
                                       decoded.channels, decoded.frame_count);

    audio_engine_.Load(std::move(decoded), track_info);
    audio_engine_.Start();

    tui_renderer_.Run(
        [&] {
          VisualFrame frame;
          frame.track_info = audio_engine_.GetTrackInfo();
          frame.playback_state = audio_engine_.GetPlaybackState();
          std::vector<float> window = audio_engine_.GetRecentMonoWindow(2048);
          frame.spectrum_bars = analyzer_.ComputeBars(window);
          frame.waveform_points = analyzer_.ComputeWaveform(window, 96);
          return frame;
        },
        [&] {
          PlaybackState state = audio_engine_.GetPlaybackState();
          return state.is_finished;
        });

    audio_engine_.Stop();
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }
}

}  // namespace vocalplayer
