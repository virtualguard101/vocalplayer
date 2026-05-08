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
  DecodedTrack DecodeFile(const std::string& path) const;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_AUDIO_DECODER_HPP_
