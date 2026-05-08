#ifndef VOCALPLAYER_SRC_UI_KEYBINDINGS_HPP_
#define VOCALPLAYER_SRC_UI_KEYBINDINGS_HPP_

namespace vocalplayer {

struct Keybindings {
  char previous_track = 'h';
  char next_track = 'l';
  char toggle_pause = ' ';
  char move_selection_up = 'k';
  char move_selection_down = 'j';
  char quit = 'q';
};

inline Keybindings DefaultKeybindings() { return Keybindings{}; }

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_UI_KEYBINDINGS_HPP_
