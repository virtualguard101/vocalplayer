/**
 * @file playlist.cpp
 * @brief Implements playlist construction from file or directory inputs.
 *
 * Key points:
 * - Validates input path existence and supported media extensions.
 * - Scans directory entries and filters playable audio files.
 * - Returns normalized absolute paths sorted for deterministic order.
 */
#include "app/playlist.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace vocalplayer {
namespace {

// Lowercase ASCII text for extension comparison.
std::string ToLower(std::string value) {
  std::transform(
      value.begin(), value.end(), value.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

// Check whether file extension is playable.
bool IsSupportedAudioFile(const std::filesystem::path& file_path) {
  static const std::vector<std::string> kSupportedExtensions = {
      ".wav", ".flac", ".mp3", ".ogg", ".opus", ".m4a", ".aac"};
  std::string extension = ToLower(file_path.extension().string());
  return std::find(kSupportedExtensions.begin(), kSupportedExtensions.end(),
                   extension) != kSupportedExtensions.end();
}

}  // namespace

// Resolve input path into a normalized sorted playlist.
std::vector<std::string> BuildPlaylist(const std::string& input_path) {
  std::filesystem::path path(input_path);
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error("Input path does not exist: " + input_path);
  }

  std::vector<std::string> playlist;
  if (std::filesystem::is_regular_file(path)) {
    if (!IsSupportedAudioFile(path)) {
      throw std::runtime_error("Unsupported audio file extension: " +
                               path.extension().string());
    }
    playlist.push_back(std::filesystem::absolute(path).string());
    return playlist;
  }

  if (!std::filesystem::is_directory(path)) {
    throw std::runtime_error("Input path must be a file or directory.");
  }

  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    const std::filesystem::path& file_path = entry.path();
    if (IsSupportedAudioFile(file_path)) {
      playlist.push_back(std::filesystem::absolute(file_path).string());
    }
  }

  std::sort(playlist.begin(), playlist.end());
  if (playlist.empty()) {
    throw std::runtime_error("No supported audio files found in directory: " +
                             input_path);
  }
  return playlist;
}

}  // namespace vocalplayer
