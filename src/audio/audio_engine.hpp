#ifndef VOCALPLAYER_SRC_AUDIO_AUDIO_ENGINE_HPP_
#define VOCALPLAYER_SRC_AUDIO_AUDIO_ENGINE_HPP_

#include <atomic>
#include <cstdint>
#include <vector>

#include "miniaudio.h"
#include "shared/types.hpp"

namespace vocalplayer {

/**
 * @brief Runtime audio playback engine backed by miniaudio output device.
 *
 * AudioEngine owns decoded PCM buffers, feeds device callbacks, and exposes
 * lightweight playback/analysis snapshots for UI and analyzer modules.
 */
class AudioEngine {
 public:
  /**
   * @brief Construct an empty engine.
   */
  AudioEngine();
  /**
   * @brief Tear down the underlying device if initialized.
   */
  ~AudioEngine();

  AudioEngine(const AudioEngine&) = delete;
  AudioEngine& operator=(const AudioEngine&) = delete;

  /**
   * @brief Load decoded PCM data and initialize playback device.
   *
   * @param decoded_track Decoded audio samples and stream format.
   * @param track_info User-facing metadata for current track.
   */
  void Load(DecodedTrack decoded_track, const TrackInfo& track_info);
  /**
   * @brief Start or restart playback from the current frame cursor.
   */
  void Start();
  /**
   * @brief Pause playback without losing current frame position.
   */
  void Pause();
  /**
   * @brief Resume playback after pause.
   */
  void Resume();
  /**
   * @brief Toggle between Pause() and Resume().
   */
  void TogglePause();
  /**
   * @brief Stop playback and uninitialize the output device.
   */
  void Stop();

  /**
   * @brief Get current playback status snapshot.
   *
   * @return PlaybackState with elapsed/duration and flags.
   */
  [[nodiscard]] PlaybackState GetPlaybackState() const;
  /**
   * @brief Extract a recent mono analysis window near current cursor.
   *
   * @param window_size Requested mono sample count.
   * @return Mono samples used by SpectrumAnalyzer.
   */
  [[nodiscard]] std::vector<float> GetRecentMonoWindow(
      uint32_t window_size) const;
  /**
   * @brief Get metadata for currently loaded track.
   *
   * @return const TrackInfo& Immutable metadata reference.
   */
  [[nodiscard]] const TrackInfo& GetTrackInfo() const { return track_info_; }

 private:
  /**
   * @brief Static bridge callback invoked by miniaudio device thread.
   */
  static void DataCallback(ma_device* device, void* output, const void* input,
                           ma_uint32 frame_count);
  /**
   * @brief Fill output PCM buffer for one callback tick.
   */
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
