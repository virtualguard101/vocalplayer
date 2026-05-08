#ifndef VOCALPLAYER_SRC_AUDIO_AUDIO_ENGINE_HPP_
#define VOCALPLAYER_SRC_AUDIO_AUDIO_ENGINE_HPP_

#include <atomic>
#include <cstdint>
#include <vector>

#include "miniaudio.h"
#include "shared/types.hpp"

namespace vocalplayer {

class AudioEngine {
 public:
  AudioEngine();
  ~AudioEngine();

  AudioEngine(const AudioEngine&) = delete;
  AudioEngine& operator=(const AudioEngine&) = delete;

  void Load(DecodedTrack decoded_track, const TrackInfo& track_info);
  void Start();
  void Pause();
  void Resume();
  void TogglePause();
  void Stop();

  [[nodiscard]] PlaybackState GetPlaybackState() const;
  [[nodiscard]] std::vector<float> GetRecentMonoWindow(
      uint32_t window_size) const;
  [[nodiscard]] const TrackInfo& GetTrackInfo() const { return track_info_; }

 private:
  static void DataCallback(ma_device* device, void* output, const void* input,
                           ma_uint32 frame_count);
  void RenderFrames(float* output, ma_uint32 frame_count);

  ma_device device_;
  bool has_device_ = false;

  DecodedTrack decoded_track_;
  TrackInfo track_info_;
  std::atomic<uint64_t> current_frame_{0};
  std::atomic<bool> is_playing_{false};
  std::atomic<bool> is_finished_{false};
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_AUDIO_AUDIO_ENGINE_HPP_
