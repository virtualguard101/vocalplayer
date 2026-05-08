#include "audio/decoder.hpp"

#include <stdexcept>
#include <string>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace vocalplayer {
namespace {

std::string BuildMiniaudioError(const std::string& action, ma_result result) {
  return action + " failed: " + std::to_string(result);
}

}  // namespace

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
  decoded.frame_count = frame_count;
  decoded.interleaved_samples.resize(frame_count * decoded.channels);

  ma_uint64 read_frames = ma_decoder_read_pcm_frames(
      &decoder, decoded.interleaved_samples.data(), frame_count, nullptr);
  ma_decoder_uninit(&decoder);

  if (read_frames == 0) {
    throw std::runtime_error("Decoded audio has zero frames.");
  }

  decoded.frame_count = read_frames;
  decoded.interleaved_samples.resize(read_frames * decoded.channels);
  return decoded;
}

}  // namespace vocalplayer
