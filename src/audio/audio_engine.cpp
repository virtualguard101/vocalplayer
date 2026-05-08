#include "audio/audio_engine.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>

namespace vocalplayer {
namespace {

std::string BuildMiniaudioError(const std::string& action, ma_result result) {
  return action + " failed: " + std::to_string(result);
}

}  // namespace

AudioEngine::AudioEngine() { std::memset(&device_, 0, sizeof(device_)); }

AudioEngine::~AudioEngine() { Stop(); }

void AudioEngine::Load(DecodedTrack decoded_track,
                       const TrackInfo& track_info) {
  Stop();

  decoded_track_ = std::move(decoded_track);
  track_info_ = track_info;
  current_frame_.store(0);
  is_playing_.store(false);
  is_finished_.store(false);

  ma_device_config config = ma_device_config_init(ma_device_type_playback);
  config.playback.format = ma_format_f32;
  config.playback.channels = decoded_track_.channels;
  config.sampleRate = decoded_track_.sample_rate_hz;
  config.dataCallback = AudioEngine::DataCallback;
  config.pUserData = this;

  ma_result result = ma_device_init(nullptr, &config, &device_);
  if (result != MA_SUCCESS) {
    throw std::runtime_error(BuildMiniaudioError("ma_device_init", result));
  }
  has_device_ = true;
}

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

void AudioEngine::Stop() {
  if (!has_device_) {
    return;
  }

  ma_device_uninit(&device_);
  has_device_ = false;
  is_playing_.store(false);
}

PlaybackState AudioEngine::GetPlaybackState() const {
  PlaybackState state;
  state.duration_sec = track_info_.duration_sec;
  state.elapsed_sec = static_cast<double>(current_frame_.load()) /
                      static_cast<double>(decoded_track_.sample_rate_hz);
  state.is_playing = is_playing_.load();
  state.is_finished = is_finished_.load();
  return state;
}

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
    mono_window[window_size - copy_frames + i] =
        sum / static_cast<float>(decoded_track_.channels);
  }
  return mono_window;
}

void AudioEngine::DataCallback(ma_device* device, void* output,
                               const void* /*input*/, ma_uint32 frame_count) {
  auto* engine = static_cast<AudioEngine*>(device->pUserData);
  engine->RenderFrames(static_cast<float*>(output), frame_count);
}

void AudioEngine::RenderFrames(float* output, ma_uint32 frame_count) {
  uint64_t start_frame = current_frame_.load();
  uint64_t total_frames = decoded_track_.frame_count;
  uint64_t channels = decoded_track_.channels;
  uint64_t remaining =
      (start_frame < total_frames) ? (total_frames - start_frame) : 0;
  uint64_t to_copy = std::min<uint64_t>(remaining, frame_count);

  if (to_copy > 0) {
    const float* src =
        decoded_track_.interleaved_samples.data() + (start_frame * channels);
    std::memcpy(output, src, to_copy * channels * sizeof(float));
  }

  if (to_copy < frame_count) {
    float* silence = output + (to_copy * channels);
    std::memset(silence, 0, (frame_count - to_copy) * channels * sizeof(float));
    is_finished_.store(true);
    is_playing_.store(false);
  }

  current_frame_.store(start_frame + to_copy);
}

}  // namespace vocalplayer
