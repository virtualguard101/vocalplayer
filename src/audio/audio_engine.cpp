/**
 * @file audio_engine.cpp
 * @brief Implements runtime playback control on top of miniaudio.
 *
 * Key points:
 * - Owns decoded PCM buffers and playback metadata for one active track.
 * - Bridges miniaudio callback thread to engine state via DataCallback().
 * - Resets playback cursor/flags on Load() and rebuilds device configuration.
 * - Exposes playback snapshots and recent mono windows for UI/analyzers.
 */

#include "audio/audio_engine.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>

namespace vocalplayer {
namespace {

// Build a readable error message from miniaudio status code.
std::string BuildMiniaudioError(const std::string& action, ma_result result) {
  return action + " failed: " + std::to_string(result);
}

}  // namespace

// Initialize device storage to a known zero state.
AudioEngine::AudioEngine() { std::memset(&device_, 0, sizeof(device_)); }

// Ensure device resources are released.
AudioEngine::~AudioEngine() { Stop(); }

// Load decoded PCM data and initialize a playback device for the stream format.
void AudioEngine::Load(DecodedTrack decoded_track,
                       const TrackInfo& track_info) {
  // Make sure the device is stopped before loading new data.
  Stop();

  // Transfer ownership of the decoded track and track info to the audio engine
  // by Move Semantics.
  decoded_track_ = std::move(decoded_track);
  track_info_ = track_info;
  // Reset the current frame, playing state, and finished state.
  current_frame_.store(0);
  is_playing_.store(false);
  is_finished_.store(false);

  // Initialize the miniaudio playback device configuration and fill it with the
  // decoded track information.
  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.format = ma_format_f32;
  config.playback.channels = decoded_track_.channels;
  config.sampleRate = decoded_track_.sample_rate_hz;
  // Set the data callback function to the AudioEngine::DataCallback function.
  config.dataCallback = AudioEngine::DataCallback;
  // Set the user data pointer to the AudioEngine instance.
  config.pUserData = this;

  // Pass playback device config to the miniaudio API to initialize the device.
  ma_result result = ma_device_init(nullptr, &config, &device_);
  if (result != MA_SUCCESS) {
    throw std::runtime_error(BuildMiniaudioError("ma_device_init", result));
  }
  // Set the has device flag to true to indicate that the device has been
  // initialized.
  has_device_ = true;
}

// Start playback from current frame cursor.
void AudioEngine::Start() {
  if (!has_device_) {
    throw std::runtime_error("Audio device is not initialized.");
  }
  ma_result result = ma_device_start(&device_);
  if (result != MA_SUCCESS) {
    throw std::runtime_error(BuildMiniaudioError("ma_device_start", result));
  }
  is_playing_.store(true);
}

// Pause playback while preserving cursor position.
void AudioEngine::Pause() {
  if (!has_device_ || !is_playing_.load()) {
    return;
  }
  ma_result result = ma_device_stop(&device_);
  if (result != MA_SUCCESS) {
    throw std::runtime_error(BuildMiniaudioError("ma_device_stop", result));
  }
  is_playing_.store(false);
}

// Resume playback after pause.
void AudioEngine::Resume() {
  if (!has_device_ || is_playing_.load() || is_finished_.load()) {
    return;
  }
  ma_result result = ma_device_start(&device_);
  if (result != MA_SUCCESS) {
    throw std::runtime_error(BuildMiniaudioError("ma_device_start", result));
  }
  is_playing_.store(true);
}

// Toggle paused/playing state.
void AudioEngine::TogglePause() {
  if (is_playing_.load()) {
    Pause();
  } else {
    Resume();
  }
}

// Stop playback and free device resources.
void AudioEngine::Stop() {
  if (!has_device_) {
    return;
  }

  ma_device_uninit(&device_);
  has_device_ = false;
  is_playing_.store(false);
}

// Build current playback status for UI and control logic.
PlaybackState AudioEngine::GetPlaybackState() const {
  PlaybackState state;
  state.duration_sec = track_info_.duration_sec;
  // Calculate the elapsed time in seconds.
  state.elapsed_sec = static_cast<double>(current_frame_.load()) /
                      static_cast<double>(decoded_track_.sample_rate_hz);
  state.is_playing = is_playing_.load();
  state.is_finished = is_finished_.load();
  return state;
}

// Extract a recent mono window for spectrum and waveform analysis.
std::vector<float> AudioEngine::GetRecentMonoWindow(
    uint32_t window_size) const {
  std::vector<float> mono_window(window_size, 0.0f);
  if (decoded_track_.frame_count == 0 || decoded_track_.channels == 0) {
    return mono_window;
  }

  uint64_t current_frame = current_frame_.load();
  uint64_t start_frame =
      (current_frame > window_size) ? (current_frame - window_size) : 0;
  uint64_t available =
      current_frame > start_frame ? current_frame - start_frame : 0;
  uint64_t copy_frames = std::min<uint64_t>(window_size, available);

  // Traverse the frames and calculate the average of the multiple channels samples.
  for (uint64_t i = 0; i < copy_frames; ++i) {
    uint64_t frame_index = start_frame + i;
    if (frame_index >= decoded_track_.frame_count) {
      break;
    }
    float sum = 0.0f;
    for (uint32_t ch = 0; ch < decoded_track_.channels; ++ch) {
      uint64_t sample_index = frame_index * decoded_track_.channels + ch;
      sum += decoded_track_.interleaved_samples[sample_index];
    }
    // Write result of the average of the multiple channels samples to the mono window.
    mono_window[window_size - copy_frames + i] =
        sum / static_cast<float>(decoded_track_.channels);
  }
  return mono_window;
}

// Static callback bridge for miniaudio device thread.
void AudioEngine::DataCallback(ma_device* device, void* output,
                               const void* /*input*/, ma_uint32 frame_count) {
  auto* engine = static_cast<AudioEngine*>(device->pUserData);
  engine->RenderFrames(static_cast<float*>(output), frame_count);
}

// Fill output buffer with decoded PCM data and trailing silence if needed.
void AudioEngine::RenderFrames(float* output, ma_uint32 frame_count) {
  uint64_t start_frame = current_frame_.load();
  uint64_t total_frames = decoded_track_.frame_count;
  uint64_t channels = decoded_track_.channels;

  // Calculate the remaining frames and the frames to copy.
  uint64_t remaining =
      (start_frame < total_frames) ? (total_frames - start_frame) : 0;

  // Calculate the frames to copy according to the remaining frames and the requested frame count.
  uint64_t to_copy = std::min<uint64_t>(remaining, frame_count);

  // Copy the decoded PCM data to the output buffer if there are remaining frames.
  if (to_copy > 0) {
    const float* src =
        decoded_track_.interleaved_samples.data() + (start_frame * channels);
    std::memcpy(output, src, to_copy * channels * sizeof(float));
  }

  // If there are remaining frames, fill the output buffer with silence(0.0f).
  if (to_copy < frame_count) {
    float* silence = output + (to_copy * channels);
    std::memset(silence, 0, (frame_count - to_copy) * channels * sizeof(float));
    is_finished_.store(true);
    is_playing_.store(false);
  }

  current_frame_.store(start_frame + to_copy);
}

}  // namespace vocalplayer
