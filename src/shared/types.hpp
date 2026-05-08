#ifndef VOCALPLAYER_SRC_SHARED_TYPES_HPP_
#define VOCALPLAYER_SRC_SHARED_TYPES_HPP_

#include <cstdint>
#include <string>
#include <vector>

namespace vocalplayer {

struct TrackInfo {
  std::string file_path;
  std::string title;
  std::string artist;
  uint32_t sample_rate_hz = 0;
  uint32_t channels = 0;
  double duration_sec = 0.0;
};

struct PlaybackState {
  double elapsed_sec = 0.0;
  double duration_sec = 0.0;
  bool is_playing = false;
  bool is_finished = false;
};

struct DecodedTrack {
  std::vector<float> interleaved_samples;
  uint32_t sample_rate_hz = 0;
  uint32_t channels = 0;
  uint64_t frame_count = 0;
};

struct VisualFrame {
  TrackInfo track_info;
  PlaybackState playback_state;
  std::vector<float> spectrum_bars;
  std::vector<float> waveform_points;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_SHARED_TYPES_HPP_
