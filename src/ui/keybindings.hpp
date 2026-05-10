/**
 * @file keybindings.hpp
 * @brief Declares configurable keyboard mappings for TUI interactions.
 *
 * Key points:
 * - Defines Keybindings as a lightweight runtime key map.
 * - Provides default Vim-like bindings via DefaultKeybindings().
 * - Keeps input mapping independent from renderer implementation details.
 */
#ifndef VOCALPLAYER_SRC_UI_KEYBINDINGS_HPP_
#define VOCALPLAYER_SRC_UI_KEYBINDINGS_HPP_

namespace vocalplayer {

/**
 * @brief Runtime keybinding map for TUI interactions.
 *
 * This struct is intentionally plain and config-friendly to support future
 * user-defined keybinding file loading.
 */
struct Keybindings {
  /// Previous track action key.
  char previous_track = 'h';
  /// Next track action key.
  char next_track = 'l';
  /// Pause/resume toggle key.
  char toggle_pause = ' ';
  /// Move playlist selection up.
  char move_selection_up = 'k';
  /// Move playlist selection down.
  char move_selection_down = 'j';
  /// Quit session key.
  char quit = 'q';
};

/**
 * @brief Build the default Vim-like keybinding profile.
 *
 * @return Keybindings Default key map.
 */
inline Keybindings DefaultKeybindings() { return Keybindings{}; }

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_UI_KEYBINDINGS_HPP_
