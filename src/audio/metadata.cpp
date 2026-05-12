/**
 * @file metadata.cpp
 * @brief Implements user-facing track metadata assembly.
 *
 * Key points:
 * - Builds TrackInfo from decoded stream attributes and source path.
 * - Derives fallback title/artist when embedded tags are unavailable.
 * - Optionally enriches metadata with TagLib when enabled at build time.
 */
#include "audio/metadata.hpp"

#include <filesystem>

#ifdef VOCALPLAYER_HAS_TAGLIB
#include <taglib/fileref.h>
#include <taglib/tag.h>
#endif

namespace vocalplayer {
namespace {

// Convert filesystem path to UTF-8 for terminal metadata display.
std::string ToUtf8String(const std::filesystem::path& path) {
  std::u8string utf8 = path.u8string();
  return std::string(utf8.begin(), utf8.end());
}

}  // namespace

// Compose UI metadata using path-based fallback plus optional TagLib tags.
TrackInfo MetadataReader::ReadTrackInfo(const std::string& file_path,
                                        uint32_t sample_rate_hz,
                                        uint32_t channels,
                                        uint64_t frame_count) const {
  TrackInfo info;
  info.file_path = file_path;
  info.sample_rate_hz = sample_rate_hz;
  info.channels = channels;
  info.duration_sec =
      static_cast<double>(frame_count) / static_cast<double>(sample_rate_hz);

  std::filesystem::path path(file_path);
  info.title = ToUtf8String(path.stem());
  info.artist = "Unknown Artist";

#ifdef VOCALPLAYER_HAS_TAGLIB
  TagLib::FileRef file_ref(file_path.c_str());
  if (!file_ref.isNull() && file_ref.tag() != nullptr) {
    TagLib::Tag* tag = file_ref.tag();
    if (!tag->title().isEmpty()) {
      info.title = tag->title().to8Bit(true);
    }
    if (!tag->artist().isEmpty()) {
      info.artist = tag->artist().to8Bit(true);
    }
  }
#endif

  return info;
}

}  // namespace vocalplayer
