/**
 * @file types.hpp
 * @brief Defines shared data contracts across decoding, playback, and UI.
 *
 * Key points:
 * - Centralizes track metadata and playback snapshot structures.
 * - Defines DecodedTrack as the PCM transport object between modules.
 * - Defines ChannelVisuals and VisualFrame as the renderer-facing view models.
 */
#ifndef VOCALPLAYER_SRC_SHARED_TYPES_HPP_
#define VOCALPLAYER_SRC_SHARED_TYPES_HPP_

#include <cstdint>
#include <string>
#include <vector>

namespace vocalplayer {

/**
 * @brief User-facing metadata for one track.
 */
struct TrackInfo {
  /// Absolute or original source file path.
  std::string file_path;
  /// Display title.
  std::string title;
  /// Display artist.
  std::string artist;
  /// Sample rate in Hz.
  uint32_t sample_rate_hz = 0;
  /// Channel count.
  uint32_t channels = 0;
  /// Total track duration in seconds.
  double duration_sec = 0.0;
};

/**
 * @brief Lightweight runtime playback snapshot.
 */
struct PlaybackState {
  /// Elapsed playback time in seconds.
  double elapsed_sec = 0.0;
  /// Total duration in seconds.
  double duration_sec = 0.0;
  /// True while device is actively outputting frames.
  bool is_playing = false;
  /// True when all frames are consumed.
  bool is_finished = false;
};

/**
 * @brief Renderer layout mode for visualization panels.
 */
enum class VisualMode {
  /// Balanced overview with all major panels visible.
  kOverview,
  /// Focus on spectrum rendering.
  kSpectrumFocus,
  /// Focus on waveform rendering.
  kWaveformFocus,
  /// Focus on meter-style numeric visualization.
  kMeterFocus,
};

/**
 * @brief Per-channel visualization snapshot for one stereo side.
 */
struct ChannelVisuals {
  /// Spectrum bar amplitudes in [0, 1].
  std::vector<float> spectrum_bars;
  /// Peak-hold spectrum marker values in [0, 1].
  std::vector<float> spectrum_peak_bars;
  /// Waveform points in [0, 1].
  std::vector<float> waveform_points;
  /// Envelope waveform points in [0, 1].
  std::vector<float> waveform_envelope_points;
  /// Root-mean-square level in [0, 1].
  float rms_level = 0.0F;
  /// Peak level in [0, 1].
  float peak_level = 0.0F;
  /// Coarse band energies in [0, 1].
  std::vector<float> band_energies;
};

/**
 * @brief Fully decoded interleaved PCM buffer.
 */
struct DecodedTrack {
  /// Interleaved float PCM samples.
  std::vector<float> interleaved_samples;
  /// Sample rate in Hz.
  uint32_t sample_rate_hz = 0;
  /// Channel count.
  uint32_t channels = 0;
  /// PCM frame count.
  uint64_t frame_count = 0;
};

/**
 * @brief UI frame contract assembled every render tick.
 */
struct VisualFrame {
  /// Metadata for current track.
  TrackInfo track_info;
  /// Playback status for current frame.
  PlaybackState playback_state;
  /// Left-channel (or mono duplicate) visualization.
  ChannelVisuals left;
  /// Right-channel (or mono duplicate) visualization.
  ChannelVisuals right;
  /// Preferred visualization mode (renderer may override by local user action).
  VisualMode visual_mode = VisualMode::kOverview;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_SHARED_TYPES_HPP_
