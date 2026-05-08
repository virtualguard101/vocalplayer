#include "app/app_controller.hpp"

#include <algorithm>
#include <atomic>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace vocalplayer {
namespace {

// Build display-friendly short names from absolute playlist paths.
std::vector<std::string> BuildTrackDisplayNames(
    const std::vector<std::string>& playlist) {
  std::vector<std::string> names;
  names.reserve(playlist.size());
  for (const std::string& path : playlist) {
    names.push_back(std::filesystem::path(path).filename().string());
  }
  return names;
}

// Clamp playlist index to a safe range.
int ClampIndex(int index, int track_count) {
  if (track_count <= 0) {
    return 0;
  }
  return std::clamp(index, 0, track_count - 1);
}

}  // namespace

// Execute the full app runtime loop (playlist + playback + UI intents).
int AppController::Run(const std::string& input_path) {
  try {
    std::vector<std::string> playlist = BuildPlaylist(input_path);
    std::vector<std::string> track_names = BuildTrackDisplayNames(playlist);
    const int total_tracks = static_cast<int>(playlist.size());

    int current_index = 0;
    while (current_index >= 0 && current_index < total_tracks) {
      const std::string& track_path = playlist[current_index];
      std::cout << "Now playing [" << current_index + 1 << "/" << total_tracks
                << "]: " << track_path << std::endl;

      std::atomic<int> selected_index{current_index};
      std::atomic<int> requested_index{current_index};
      std::atomic<bool> switch_requested{false};
      std::atomic<bool> exit_requested{false};

      try {
        DecodedTrack decoded = decoder_.DecodeFile(track_path);
        TrackInfo track_info = metadata_reader_.ReadTrackInfo(
            track_path, decoded.sample_rate_hz, decoded.channels,
            decoded.frame_count);

        audio_engine_.Load(std::move(decoded), track_info);
        audio_engine_.Start();

        tui_renderer_.Run(
            [&] {
              VisualFrame frame;
              frame.track_info = audio_engine_.GetTrackInfo();
              frame.playback_state = audio_engine_.GetPlaybackState();
              std::vector<float> window =
                  audio_engine_.GetRecentMonoWindow(2048);
              frame.spectrum_bars = analyzer_.ComputeBars(window);
              frame.waveform_points = analyzer_.ComputeWaveform(window, 96);
              return frame;
            },
            [&] {
              PlaylistViewModel view_model;
              view_model.tracks = track_names;
              view_model.current_track_index = current_index;
              view_model.selected_track_index =
                  ClampIndex(selected_index.load(), total_tracks);
              return view_model;
            },
            [&](UiIntent intent) {
              switch (intent) {
                case UiIntent::kQuit:
                  exit_requested.store(true);
                  switch_requested.store(true);
                  break;
                case UiIntent::kPreviousTrack:
                  requested_index.store(
                      ClampIndex(current_index - 1, total_tracks));
                  switch_requested.store(true);
                  break;
                case UiIntent::kNextTrack:
                  requested_index.store(
                      ClampIndex(current_index + 1, total_tracks));
                  switch_requested.store(true);
                  break;
                case UiIntent::kTogglePause:
                  audio_engine_.TogglePause();
                  break;
                case UiIntent::kPlaySelectedTrack:
                  requested_index.store(
                      ClampIndex(selected_index.load(), total_tracks));
                  switch_requested.store(true);
                  break;
              }
            },
            [&](int next_selected_index) {
              selected_index.store(
                  ClampIndex(next_selected_index, total_tracks));
            },
            [&] {
              PlaybackState state = audio_engine_.GetPlaybackState();
              return state.is_finished || switch_requested.load();
            });
      } catch (const std::exception& ex) {
        audio_engine_.Stop();
        std::cerr << "Warning: skip track due to error: " << ex.what()
                  << std::endl;
        current_index += 1;
        continue;
      }

      PlaybackState state = audio_engine_.GetPlaybackState();
      audio_engine_.Stop();

      if (exit_requested.load()) {
        return 0;
      }

      if (switch_requested.load()) {
        current_index = requested_index.load();
        continue;
      }

      if (state.is_finished) {
        current_index += 1;
      } else {
        return 0;
      }
    }

    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }
}

}  // namespace vocalplayer
