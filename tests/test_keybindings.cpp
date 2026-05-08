#include <cassert>

#include "ui/keybindings.hpp"

int main() {
  vocalplayer::Keybindings keybindings = vocalplayer::DefaultKeybindings();

  assert(keybindings.previous_track == 'h');
  assert(keybindings.next_track == 'l');
  assert(keybindings.toggle_pause == ' ');
  assert(keybindings.move_selection_up == 'k');
  assert(keybindings.move_selection_down == 'j');
  assert(keybindings.quit == 'q');

  vocalplayer::Keybindings custom;
  custom.previous_track = 'a';
  custom.next_track = 'd';
  assert(custom.previous_track == 'a');
  assert(custom.next_track == 'd');

  return 0;
}
