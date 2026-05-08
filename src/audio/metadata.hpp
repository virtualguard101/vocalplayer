#ifndef VOCALPLAYER_SRC_AUDIO_METADATA_HPP_
#define VOCALPLAYER_SRC_AUDIO_METADATA_HPP_

#include <string>

#include "shared/types.hpp"

namespace vocalplayer {

class MetadataReader {
 public:
  TrackInfo ReadTrackInfo(const std::string& file_path, uint32_t sample_rate_hz,
                          uint32_t channels, uint64_t frame_count) const;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_AUDIO_METADATA_HPP_
