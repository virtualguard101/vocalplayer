#ifndef VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_
#define VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_

#include <functional>
#include <string>
#include <vector>

#include "shared/types.hpp"
#include "ui/keybindings.hpp"

namespace vocalplayer {

enum class UiIntent {
  kQuit,
  kPreviousTrack,
  kNextTrack,
  kTogglePause,
  kPlaySelectedTrack,
};

struct PlaylistViewModel {
  std::vector<std::string> tracks;
  int current_track_index = 0;
  int selected_track_index = 0;
};

class TuiRenderer {
 public:
  void Run(const std::function<VisualFrame()>& frame_provider,
           const std::function<PlaylistViewModel()>& playlist_provider,
           const std::function<void(UiIntent)>& on_intent,
           const std::function<void(int)>& on_selection_changed,
           const std::function<bool()>& should_stop);

 private:
  static std::string BuildProgressBar(float ratio, int width);
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_
