#include "app/app_controller.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace vocalplayer {

int AppController::Run(const std::string& input_path) {
  try {
    std::vector<std::string> playlist = BuildPlaylist(input_path);
    int played_tracks = 0;
    for (int i = 0; i < static_cast<int>(playlist.size()); ++i) {
      const std::string& track_path = playlist[i];
      bool should_continue =
          PlaySingleTrack(track_path, i + 1, static_cast<int>(playlist.size()));
      if (!should_continue) {
        return 0;
      }
      ++played_tracks;
    }
    if (played_tracks == 0) {
      std::cerr << "Error: no track could be played." << std::endl;
      return 1;
    }
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }
}

bool AppController::PlaySingleTrack(const std::string& track_path,
                                    int current_index, int total_tracks) {
  try {
    std::cout << "Now playing [" << current_index << "/" << total_tracks
              << "]: " << track_path << std::endl;
    DecodedTrack decoded = decoder_.DecodeFile(track_path);
    TrackInfo track_info =
        metadata_reader_.ReadTrackInfo(track_path, decoded.sample_rate_hz,
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

    PlaybackState state = audio_engine_.GetPlaybackState();
    audio_engine_.Stop();
    return state.is_finished;
  } catch (const std::exception& ex) {
    audio_engine_.Stop();
    std::cerr << "Warning: skip track due to error: " << ex.what() << std::endl;
    return true;
  }
}

}  // namespace vocalplayer
