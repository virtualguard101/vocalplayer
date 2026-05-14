/**
 * @file tui_renderer.hpp
 * @brief Declares terminal UI contracts and renderer entry points.
 *
 * Key points:
 * - Defines UiIntent and PlaylistViewModel for controller/UI boundaries.
 * - Exposes TuiRenderer::Run() for one interactive session lifecycle.
 * - Uses VisualUpdatePipeline for worker-thread frame production and coalesced
 *   FTXUI redraw scheduling; playlist_provider runs on the main draw path.
 */
#ifndef VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_
#define VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_

#include <functional>
#include <string>
#include <vector>

#include "shared/types.hpp"
#include "ui/keybindings.hpp"
#include "ui/theme.hpp"

namespace vocalplayer {

/**
 * @brief High-level UI intent emitted by TUI input handling.
 */
enum class UiIntent {
  /// Quit the current player session.
  kQuit,
  /// Switch to previous track.
  kPreviousTrack,
  /// Switch to next track.
  kNextTrack,
  /// Pause/resume current playback.
  kTogglePause,
  /// Play the currently selected playlist item.
  kPlaySelectedTrack,
};

/**
 * @brief UI-facing playlist state snapshot.
 */
struct PlaylistViewModel {
  /// Display names for playlist tracks.
  std::vector<std::string> tracks;
  /// Currently playing track index.
  int current_track_index = 0;
  /// Cursor-selected track index.
  int selected_track_index = 0;
};

/**
 * @brief Cross-track UI session preferences persisted by AppController.
 */
struct UiSessionState {
  /// Active built-in color theme.
  ThemeId theme_id = ThemeId::kDefault;
  /// Active visualization layout mode.
  VisualMode visual_mode = VisualMode::kOverview;
  /// Whether envelope waveform rendering is enabled.
  bool use_envelope_waveform = false;
};

/**
 * @brief Terminal renderer and input event bridge.
 *
 * TuiRenderer renders track/analysis/playlist views and converts keyboard and
 * mouse events into UiIntent callbacks for AppController.
 */
class TuiRenderer {
 public:
  /**
   * @brief Run one interactive rendering session.
   *
   * @param frame_provider Supplies visual/audio frame snapshots; invoked from a
   *        dedicated worker thread inside this session (see
   * VisualUpdatePipeline).
   * @param playlist_provider Supplies latest playlist view model.
   * @param on_intent Callback for high-level playback intents.
   * @param on_selection_changed Callback for playlist cursor updates.
   * @param should_stop Predicate for external stop condition.
   *
   * @note This function blocks until quit, track-switch intent, or stop
   * condition.
   */
  void Run(const std::function<VisualFrame()>& frame_provider,
           const std::function<PlaylistViewModel()>& playlist_provider,
           const std::function<void(UiIntent)>& on_intent,
           const std::function<void(int)>& on_selection_changed,
           const std::function<bool()>& should_stop,
           UiSessionState* session_state = nullptr);

 private:
  /**
   * @brief Build textual progress bar representation.
   *
   * @param ratio Playback ratio in [0, 1].
   * @param width Bar width in characters.
   * @return std::string Fixed-width progress bar.
   */
  static std::string BuildProgressBar(float ratio, int width);
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_UI_TUI_RENDERER_HPP_
