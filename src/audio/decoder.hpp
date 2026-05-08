#ifndef VOCALPLAYER_SRC_AUDIO_DECODER_HPP_
#define VOCALPLAYER_SRC_AUDIO_DECODER_HPP_

#include <string>

#include "shared/types.hpp"

namespace vocalplayer {

class Decoder {
 public:
  DecodedTrack DecodeFile(const std::string& path) const;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_AUDIO_DECODER_HPP_
