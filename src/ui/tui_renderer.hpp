#ifndef VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_
#define VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_

#include <atomic>
#include <functional>

#include "shared/types.hpp"

namespace vocalplayer {

class TuiRenderer {
 public:
  void Run(const std::function<VisualFrame()>& frame_provider,
           const std::function<bool()>& should_stop);

 private:
  static std::string BuildProgressBar(float ratio, int width);
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_
