/**
 * @file metadata.hpp
 * @brief Declares metadata reader interfaces for UI-facing track details.
 *
 * Key points:
 * - Defines MetadataReader contract for building TrackInfo objects.
 * - Accepts decoded stream fields to compute duration and technical info.
 * - Keeps metadata extraction separate from decoding/playback modules.
 */
#ifndef VOCALPLAYER_SRC_AUDIO_METADATA_HPP_
#define VOCALPLAYER_SRC_AUDIO_METADATA_HPP_

#include <string>

#include "shared/types.hpp"

namespace vocalplayer {

/**
 * @brief Read user-facing metadata for one decoded track.
 */
class MetadataReader {
 public:
  /**
   * @brief Build TrackInfo from file path and decoded stream attributes.
   *
   * @param file_path Source file path.
   * @param sample_rate_hz Decoded sample rate.
   * @param channels Decoded channel count.
   * @param frame_count Decoded frame count.
   * @return TrackInfo for UI rendering and playback state.
   *
   * @note TagLib is optional; when unavailable, title/artist fall back to
   * filename and default artist.
   */
  TrackInfo ReadTrackInfo(const std::string& file_path, uint32_t sample_rate_hz,
                          uint32_t channels, uint64_t frame_count) const;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_AUDIO_METADATA_HPP_
