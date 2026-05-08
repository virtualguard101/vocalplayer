#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "app/playlist.hpp"

int main() {
  namespace fs = std::filesystem;

  fs::path temp_root =
      fs::temp_directory_path() / "vocalplayer_playlist_test_workspace";
  fs::remove_all(temp_root);
  fs::create_directories(temp_root);

  std::ofstream(temp_root / "b_track.flac").put('\n');
  std::ofstream(temp_root / "a_track.wav").put('\n');
  std::ofstream(temp_root / "notes.txt").put('\n');

  std::vector<std::string> playlist =
      vocalplayer::BuildPlaylist(temp_root.string());
  assert(playlist.size() == 2);
  assert(fs::path(playlist[0]).filename().string() == "a_track.wav");
  assert(fs::path(playlist[1]).filename().string() == "b_track.flac");

  std::vector<std::string> single =
      vocalplayer::BuildPlaylist((temp_root / "a_track.wav").string());
  assert(single.size() == 1);
  assert(fs::path(single[0]).filename().string() == "a_track.wav");

  fs::remove_all(temp_root);
  return 0;
}
