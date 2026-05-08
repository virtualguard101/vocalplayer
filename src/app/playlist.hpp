#ifndef VOCALPLAYER_SRC_APP_PLAYLIST_HPP_
#define VOCALPLAYER_SRC_APP_PLAYLIST_HPP_

#include <string>
#include <vector>

namespace vocalplayer {

/**
 * @brief Build a playable track list from input path.
 *
 * @param input_path Path to a single audio file or a directory.
 * @return Sorted absolute file path list for supported audio files.
 *
 * @note Throws std::runtime_error when the path is invalid or no supported
 * files are found.
 */
std::vector<std::string> BuildPlaylist(const std::string& input_path);

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_APP_PLAYLIST_HPP_
