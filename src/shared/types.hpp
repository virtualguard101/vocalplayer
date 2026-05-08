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
  /// Spectrum bar amplitudes in [0, 1].
  std::vector<float> spectrum_bars;
  /// Waveform points in [0, 1].
  std::vector<float> waveform_points;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_SHARED_TYPES_HPP_
