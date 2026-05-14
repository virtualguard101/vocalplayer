/**
 * @file app_controller.cpp
 * @brief Implements the top-level runtime orchestration for playback sessions.
 *
 * Key points:
 * - Builds playlist context and drives per-track playback loop.
 * - Coordinates Decoder, MetadataReader, AudioEngine, and TuiRenderer.
 * - Handles UI intents for pause, quit, and track switching.
 */
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

constexpr uint32_t kAnalysisWindowSize = 2048;
constexpr uint32_t kWaveformPointCount = 96;
constexpr uint32_t kBandEnergyCount = 3;
constexpr float kPeakDecayPerFrame = 0.035f;

/**
 * @brief Run spectrum/waveform analysis for one channel window.
 *
 * @param analyzer Analyzer instance shared across channels.
 * @param out Per-channel output fields to populate.
 * @param window Time-domain samples for this channel.
 * @param spectrum_peaks Mutable peak-hold state for this channel.
 * @param waveform_point_count Waveform decimation target.
 * @param band_energy_count Number of coarse band energy bins.
 * @param peak_decay Per-frame decay applied to held spectrum peaks.
 */
void FillChannelVisuals(SpectrumAnalyzer& analyzer, ChannelVisuals& out,
                        const std::vector<float>& window,
                        std::vector<float>& spectrum_peaks,
                        uint32_t waveform_point_count,
                        uint32_t band_energy_count, float peak_decay) {
  out.spectrum_bars = analyzer.ComputeBars(window);
  if (spectrum_peaks.size() != out.spectrum_bars.size()) {
    spectrum_peaks.assign(out.spectrum_bars.size(), 0.0f);
  }
  for (size_t i = 0; i < out.spectrum_bars.size(); ++i) {
    const float decayed_peak = std::max(0.0f, spectrum_peaks[i] - peak_decay);
    spectrum_peaks[i] = std::max(out.spectrum_bars[i], decayed_peak);
  }
  out.spectrum_peak_bars = spectrum_peaks;
  out.waveform_points = analyzer.ComputeWaveform(window, waveform_point_count);
  out.waveform_envelope_points =
      analyzer.ComputeWaveformEnvelope(window, waveform_point_count);
  const AudioLevels levels = analyzer.ComputeLevels(window);
  out.rms_level = levels.rms_level;
  out.peak_level = levels.peak_level;
  out.band_energies = analyzer.ComputeBandEnergies(window, band_energy_count);
}

/**
 * @brief Build one visualization snapshot from the current playback cursor.
 *
 * @param engine Active audio engine for PCM windows and playback metadata.
 * @param analyzer Shared spectrum analyzer instance.
 * @param peaks_l Mutable peak-hold state for the left channel spectrum.
 * @param peaks_r Mutable peak-hold state for the right channel spectrum.
 * @return Populated VisualFrame for the UI pipeline.
 */
VisualFrame BuildPlaybackVisualFrame(AudioEngine& engine,
                                     SpectrumAnalyzer& analyzer,
                                     std::vector<float>& peaks_l,
                                     std::vector<float>& peaks_r) {
  VisualFrame frame;
  frame.track_info = engine.GetTrackInfo();
  frame.playback_state = engine.GetPlaybackState();
  std::vector<float> window_l =
      engine.GetRecentChannelWindow(0, kAnalysisWindowSize);
  std::vector<float> window_r =
      (frame.track_info.channels >= 2)
          ? engine.GetRecentChannelWindow(1, kAnalysisWindowSize)
          : window_l;
  FillChannelVisuals(analyzer, frame.left, window_l, peaks_l,
                     kWaveformPointCount, kBandEnergyCount, kPeakDecayPerFrame);
  FillChannelVisuals(analyzer, frame.right, window_r, peaks_r,
                     kWaveformPointCount, kBandEnergyCount, kPeakDecayPerFrame);
  frame.visual_mode = VisualMode::kOverview;
  return frame;
}

// Convert filesystem path to UTF-8 text for terminal rendering.
std::string ToUtf8String(const std::filesystem::path& path) {
  std::u8string utf8 = path.u8string();
  return std::string(utf8.begin(), utf8.end());
}

// Build display-friendly short names from absolute playlist paths.
std::vector<std::string> BuildTrackDisplayNames(
    const std::vector<std::string>& playlist) {
  std::vector<std::string> names;
  names.reserve(playlist.size());
  for (const std::string& path : playlist) {
    names.push_back(ToUtf8String(std::filesystem::path(path).filename()));
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
    // Parse the input path and build the playlist.
    std::vector<std::string> playlist = BuildPlaylist(input_path);
    // Generate the display names for UI.
    std::vector<std::string> track_names = BuildTrackDisplayNames(playlist);
    const int total_tracks = static_cast<int>(playlist.size());
    UiSessionState ui_session_state;

    int current_index = 0;
    // Playback loop for each track in the playlist.
    while (current_index >= 0 && current_index < total_tracks) {
      const std::string& track_path = playlist[current_index];
      std::cout << "Now playing [" << current_index + 1 << "/" << total_tracks
                << "]: " << track_path << '\n';

      std::atomic<int> selected_index{current_index};
      std::atomic<int> requested_index{current_index};
      std::atomic<bool> switch_requested{false};
      std::atomic<bool> exit_requested{false};
      std::vector<float> spectrum_peaks_l;
      std::vector<float> spectrum_peaks_r;

      // Singal track playback process
      try {
        // Get the decoded track from the decoder.
        DecodedTrack decoded = decoder_.DecodeFile(track_path);

        // Get the metadata of each track so that can be rendered in the UI
        // later.
        TrackInfo track_info = metadata_reader_.ReadTrackInfo(
            track_path, decoded.sample_rate_hz, decoded.channels,
            decoded.frame_count);

        // Load the decoded track and the track info to the audio engine
        // instance and start the playback.
        audio_engine_.Load(std::move(decoded), track_info);
        audio_engine_.Start();

        // Run one interactive UI session with five callbacks:
        // frame provider, playlist provider, intent handler,
        // selection sync, and stop predicate.
        tui_renderer_.Run(
            [&] {
              return BuildPlaybackVisualFrame(
                  audio_engine_, analyzer_, spectrum_peaks_l, spectrum_peaks_r);
            },
            // Build the playlist view model (tracks/current/selection).
            [&] {
              PlaylistViewModel view_model;
              view_model.tracks = track_names;
              view_model.current_track_index = current_index;
              view_model.selected_track_index =
                  ClampIndex(selected_index.load(), total_tracks);
              return view_model;
            },
            // Handle high-level UI intents and mutate control state.
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
            // Keep the controller-side selected index in sync with UI cursor.
            [&](int next_selected_index) {
              selected_index.store(
                  ClampIndex(next_selected_index, total_tracks));
            },
            // Exit the UI loop when playback completes or a track switch is
            // requested.
            [&] {
              PlaybackState state = audio_engine_.GetPlaybackState();
              return state.is_finished || switch_requested.load();
            },
            &ui_session_state);
      } catch (const std::exception& ex) {
        audio_engine_.Stop();
        std::cerr << "Warning: skip track due to error: " << ex.what() << '\n';
        current_index += 1;
        continue;
      }

      // Get the playback state and stop the playback so that audio engine
      // resources can be released after the UI session ends.
      PlaybackState state = audio_engine_.GetPlaybackState();
      audio_engine_.Stop();

      // Exit flag for whole application.
      if (exit_requested.load()) {
        return 0;
      }

      // Switch to the requested track.
      if (switch_requested.load()) {
        current_index = requested_index.load();
        continue;
      }

      // Playback next track or exit the application because of external
      // request.
      if (state.is_finished) {
        current_index += 1;
      } else {
        return 0;
      }
    }

    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    return 1;
  }
}

}  // namespace vocalplayer
