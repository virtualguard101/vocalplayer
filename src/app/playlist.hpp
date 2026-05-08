#ifndef VOCALPLAYER_SRC_APP_PLAYLIST_HPP_
#define VOCALPLAYER_SRC_APP_PLAYLIST_HPP_

#include <string>
#include <vector>

namespace vocalplayer {

std::vector<std::string> BuildPlaylist(const std::string& input_path);

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_APP_PLAYLIST_HPP_
