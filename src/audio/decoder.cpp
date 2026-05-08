#include "audio/decoder.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace vocalplayer {
namespace {

// Build a readable error message from miniaudio status code.
std::string BuildMiniaudioError(const std::string& action, ma_result result) {
  return action + " failed: " + std::to_string(result);
}

}  // namespace

// Decode file to interleaved PCM float samples.
DecodedTrack Decoder::DecodeFile(const std::string& path) const {
  ma_decoder_config config =
      ma_decoder_config_init(ma_format_f32, 0, 0);  // Native channels/rate.

  ma_decoder decoder;
  ma_result result = ma_decoder_init_file(path.c_str(), &config, &decoder);
  if (result != MA_SUCCESS) {
    throw std::runtime_error(
        BuildMiniaudioError("ma_decoder_init_file", result));
  }

  ma_uint64 frame_count = 0;
  result = ma_decoder_get_length_in_pcm_frames(&decoder, &frame_count);
  if (result != MA_SUCCESS) {
    ma_decoder_uninit(&decoder);
    throw std::runtime_error(
        BuildMiniaudioError("ma_decoder_get_length_in_pcm_frames", result));
  }

  DecodedTrack decoded;
  decoded.channels = decoder.outputChannels;
  decoded.sample_rate_hz = decoder.outputSampleRate;
  if (decoded.channels == 0 || decoded.sample_rate_hz == 0) {
    ma_decoder_uninit(&decoder);
    throw std::runtime_error("Decoder returned invalid stream format.");
  }

  if (frame_count > 0) {
    decoded.interleaved_samples.resize(frame_count * decoded.channels);
    ma_uint64 read_frames = 0;
    result =
        ma_decoder_read_pcm_frames(&decoder, decoded.interleaved_samples.data(),
                                   frame_count, &read_frames);
    if (result != MA_SUCCESS && result != MA_AT_END) {
      ma_decoder_uninit(&decoder);
      throw std::runtime_error(
          BuildMiniaudioError("ma_decoder_read_pcm_frames", result));
    }
    decoded.interleaved_samples.resize(read_frames * decoded.channels);
  } else {
    constexpr ma_uint64 kChunkFrames = 4096;
    std::vector<float> chunk(kChunkFrames * decoded.channels, 0.0f);
    while (true) {
      ma_uint64 read_frames = 0;
      result = ma_decoder_read_pcm_frames(&decoder, chunk.data(), kChunkFrames,
                                          &read_frames);
      if (result != MA_SUCCESS && result != MA_AT_END) {
        ma_decoder_uninit(&decoder);
        throw std::runtime_error(
            BuildMiniaudioError("ma_decoder_read_pcm_frames", result));
      }
      if (read_frames == 0) {
        break;
      }
      size_t copy_samples = static_cast<size_t>(read_frames * decoded.channels);
      decoded.interleaved_samples.insert(decoded.interleaved_samples.end(),
                                         chunk.begin(),
                                         chunk.begin() + copy_samples);
    }
  }

  decoded.frame_count = static_cast<ma_uint64>(
      decoded.interleaved_samples.size() / decoded.channels);
  ma_decoder_uninit(&decoder);

  if (decoded.frame_count == 0) {
    throw std::runtime_error(
        "Decoded audio has zero frames. The file may be empty or unsupported.");
  }

  return decoded;
}

}  // namespace vocalplayer
