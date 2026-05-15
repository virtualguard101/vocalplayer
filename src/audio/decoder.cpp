/**
 * @file decoder.cpp
 * @brief Implements compressed-audio decoding into interleaved float PCM.
 *
 * Key points:
 * - Uses miniaudio decoder APIs to open files and read PCM frames.
 * - Handles both known-length and unknown-length streams efficiently.
 * - Produces DecodedTrack with samples, channel count, and sample rate.
 */
#include "audio/decoder.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "platform/miniaudio_decoder_open.hpp"

namespace vocalplayer {
namespace {

// Build a readable error message from miniaudio status code.
std::string BuildMiniaudioError(const std::string& action, ma_result result) {
  return action + " failed: " + std::to_string(result);
}

}  // namespace

// Decode file to interleaved PCM float samples.
DecodedTrack Decoder::DecodeFile(const std::string& path) {
  // Initialize decoder output as float PCM by miniaudio API; use source-native
  // channels/rate.
  ma_decoder_config config =
      ma_decoder_config_init(ma_format_f32, 0, 0);  // Native channels/rate.

  ma_decoder decoder;
  // Open file and bind it to a miniaudio decoder instance.
  ma_result result =
      platform::MaDecoderInitFromUtf8Path(&decoder, &config, path.c_str());
  if (result != MA_SUCCESS) {
    throw std::runtime_error(
        BuildMiniaudioError("MaDecoderInitFromUtf8Path", result));
  }

  ma_uint64 frame_count = 0;
  // Query total frame count to decide between bulk-read and chunked-read path.
  result = ma_decoder_get_length_in_pcm_frames(&decoder, &frame_count);
  if (result != MA_SUCCESS) {
    ma_decoder_uninit(&decoder);
    throw std::runtime_error(
        BuildMiniaudioError("ma_decoder_get_length_in_pcm_frames", result));
  }

  DecodedTrack decoded;
  // Capture the actual output stream format resolved by the decoder.
  decoded.channels = decoder.outputChannels;
  decoded.sample_rate_hz = decoder.outputSampleRate;
  if (decoded.channels == 0 || decoded.sample_rate_hz == 0) {
    ma_decoder_uninit(&decoder);
    throw std::runtime_error("Decoder returned invalid stream format.");
  }

  if (frame_count > 0) {
    // Known length: pre-allocate once and read as many frames as available.
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
    // Unknown length: stream in fixed-size chunks until end-of-file.
    constexpr ma_uint64 kChunkFrames = 4096;
    std::vector<float> chunk(kChunkFrames * decoded.channels, 0.0F);
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
        // No more decoded frames available.
        break;
      }
      const auto copy_samples =
          static_cast<std::ptrdiff_t>(read_frames * decoded.channels);
      decoded.interleaved_samples.insert(decoded.interleaved_samples.end(),
                                         chunk.begin(),
                                         chunk.begin() + copy_samples);
    }
  }

  decoded.frame_count = static_cast<ma_uint64>(
      decoded.interleaved_samples.size() / decoded.channels);
  // Always release decoder resources before returning or throwing.
  ma_decoder_uninit(&decoder);

  if (decoded.frame_count == 0) {
    // Treat empty decode result as invalid input/unsupported media.
    throw std::runtime_error(
        "Decoded audio has zero frames. The file may be empty or unsupported.");
  }

  return decoded;
}

}  // namespace vocalplayer
