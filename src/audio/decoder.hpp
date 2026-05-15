/**
 * @file decoder.hpp
 * @brief Declares audio decoder interfaces that convert sources to PCM.
 *
 * Key points:
 * - Provides Decoder as the primary decoding abstraction.
 * - Defines DecodeFile() contract returning DecodedTrack.
 * - Keeps room for alternate backends such as FFmpegDecoder.
 */
#ifndef VOCALPLAYER_SRC_AUDIO_DECODER_HPP_
#define VOCALPLAYER_SRC_AUDIO_DECODER_HPP_

#include <string>

#include "shared/types.hpp"

namespace vocalplayer {

/**
 * @brief Decode compressed audio files into PCM float frames.
 */
class Decoder {
 public:
  /**
   * @brief Decode one audio file into interleaved float PCM samples.
   *
   * @param path Input audio file path.
   * @return DecodedTrack containing samples and stream format.
   *
   * @note Uses miniaudio decoder and supports fallback chunk reads when the
   * stream length is unknown.
   */
  static DecodedTrack DecodeFile(const std::string& path);
};

}  // namespace vocalplayer

class FFmpegDecoder {
 public:
  // TODO: Introducing a parallel implementation of a decoder based on the
  // FFmpeg API.
};

#endif  // VOCALPLAYER_SRC_AUDIO_DECODER_HPP_
