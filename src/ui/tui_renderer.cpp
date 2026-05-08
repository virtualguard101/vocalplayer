#include "ui/tui_renderer.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/mouse.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"

namespace vocalplayer {
namespace {

using namespace ftxui;  // NOLINT

constexpr int kPlaylistVisibleRows = 10;
constexpr int kPlaylistStartY = 16;

// Build a one-column spectrum bar using repeated characters.
std::string BuildBar(float value, int height) {
  int filled = static_cast<int>(value * static_cast<float>(height));
  return std::string(std::max(filled, 0), '#');
}

// Render normalized spectrum bars into a fixed-height row block.
Element RenderSpectrum(const std::vector<float>& bars) {
  std::vector<Element> cols;
  cols.reserve(bars.size());
  constexpr int kMaxBarHeight = 8;
  for (float value : bars) {
    cols.push_back(vbox({
        filler(),
        text(BuildBar(value, kMaxBarHeight)) | color(Color::Cyan),
    }));
  }
  return hbox(std::move(cols)) | size(HEIGHT, EQUAL, kMaxBarHeight);
}

// Render waveform points as lightweight ASCII intensity glyphs.
Element RenderWaveform(const std::vector<float>& wave) {
  std::vector<Element> points;
  points.reserve(wave.size());
  for (float v : wave) {
    const int level = static_cast<int>(v * 7.0f);
    const char glyph = " .:-=+*#"[std::clamp(level, 0, 7)];
    points.push_back(text(std::string(1, glyph)) | color(Color::Magenta));
  }
  return hbox(std::move(points));
}

// Clamp playlist viewport start offset into valid range.
int ClampOffset(int offset, int track_count, int visible_rows) {
  int max_offset = std::max(0, track_count - visible_rows);
  return std::clamp(offset, 0, max_offset);
}

// Scroll viewport only when selection goes out of visible window.
int FollowSelectedOffset(int selected_index, int offset, int visible_rows) {
  if (selected_index < offset) {
    return selected_index;
  }
  if (selected_index >= offset + visible_rows) {
    return selected_index - visible_rows + 1;
  }
  return offset;
}

// Render a windowed playlist view with optional leading/trailing ellipsis rows.
Element RenderPlaylist(const PlaylistViewModel& playlist, int view_offset,
                       int visible_rows) {
  view_offset = ClampOffset(
      view_offset, static_cast<int>(playlist.tracks.size()), visible_rows);
  int end_index = std::min(static_cast<int>(playlist.tracks.size()),
                           view_offset + visible_rows);
  std::vector<Element> rows;
  rows.reserve(std::max(visible_rows + 2, 3));
  if (view_offset > 0) {
    rows.push_back(text("..."));
  }
  for (int i = view_offset; i < end_index; ++i) {
    std::string prefix = "  ";
    if (i == playlist.current_track_index) {
      prefix = "> ";
    } else if (i == playlist.selected_track_index) {
      prefix = "* ";
    }
    Element row =
        text(prefix + std::to_string(i + 1) + ". " + playlist.tracks[i]);
    if (i == playlist.current_track_index) {
      row = row | bold | color(Color::Green);
    } else if (i == playlist.selected_track_index) {
      row = row | color(Color::Yellow);
    }
    rows.push_back(std::move(row));
  }
  if (end_index < static_cast<int>(playlist.tracks.size())) {
    rows.push_back(text("..."));
  }
  if (playlist.tracks.empty()) {
    rows.push_back(text("(empty playlist)"));
  }
  return vbox(std::move(rows)) | size(HEIGHT, LESS_THAN, visible_rows + 2);
}

// Render highlighted selection status line for pending vs live target.
Element RenderSelectionStatus(const PlaylistViewModel& playlist) {
  if (playlist.tracks.empty()) {
    return text("Selected: none (playlist is empty)") | color(Color::GrayDark);
  }

  const bool is_pending =
      playlist.selected_track_index != playlist.current_track_index;
  const std::string status_label = is_pending ? "PENDING" : "LIVE";
  const Color status_color = is_pending ? Color::Yellow : Color::Green;
  const std::string selected_text =
      std::to_string(playlist.selected_track_index + 1) + "/" +
      std::to_string(playlist.tracks.size());

  return hbox({
      text("Selected "),
      text(selected_text) | bold | color(Color::Cyan),
      text("  "),
      text("[" + status_label + "]") | bold | color(status_color),
      text("  "),
      text("(Press Enter to play)"),
  });
}

}  // namespace

// Run one interactive TUI session and map raw input to playback intents.
void TuiRenderer::Run(
    const std::function<VisualFrame()>& frame_provider,
    const std::function<PlaylistViewModel()>& playlist_provider,
    const std::function<void(UiIntent)>& on_intent,
    const std::function<void(int)>& on_selection_changed,
    const std::function<bool()>& should_stop) {
  VisualFrame latest_frame = frame_provider();
  PlaylistViewModel latest_playlist = playlist_provider();
  std::atomic<bool> keep_running = true;
  Keybindings keybindings = DefaultKeybindings();

  auto screen = ScreenInteractive::Fullscreen();
  int track_count = static_cast<int>(latest_playlist.tracks.size());
  int initial_selected = 0;
  if (track_count > 0) {
    initial_selected =
        std::clamp(latest_playlist.selected_track_index, 0, track_count - 1);
  }
  int initial_offset =
      FollowSelectedOffset(initial_selected, 0, kPlaylistVisibleRows);
  initial_offset =
      ClampOffset(initial_offset, track_count, kPlaylistVisibleRows);
  std::atomic<int> selected_index{initial_selected};
  std::atomic<int> view_offset{initial_offset};

  auto select_delta = [&](int delta) {
    if (latest_playlist.tracks.empty()) {
      return;
    }
    int current_selected = selected_index.load();
    int next_selected =
        std::clamp(current_selected + delta, 0,
                   static_cast<int>(latest_playlist.tracks.size()) - 1);
    selected_index.store(next_selected);
    int next_offset = FollowSelectedOffset(next_selected, view_offset.load(),
                                           kPlaylistVisibleRows);
    next_offset = ClampOffset(next_offset,
                              static_cast<int>(latest_playlist.tracks.size()),
                              kPlaylistVisibleRows);
    view_offset.store(next_offset);
    on_selection_changed(next_selected);
  };

  auto play_selected = [&] {
    if (latest_playlist.tracks.empty()) {
      return;
    }
    int next_selected = selected_index.load();
    on_selection_changed(next_selected);
    on_intent(UiIntent::kPlaySelectedTrack);
    keep_running.store(false);
    screen.ExitLoopClosure()();
  };

  auto component = Renderer([&] {
    const PlaybackState& state = latest_frame.playback_state;
    float ratio = 0.0f;
    if (state.duration_sec > 0.0) {
      ratio = static_cast<float>(state.elapsed_sec / state.duration_sec);
    }

    return vbox({
               text("VocalPlayer MVP") | bold,
               separator(),
               text("Title: " + latest_frame.track_info.title),
               text("Artist: " + latest_frame.track_info.artist),
               text(std::string("State: ") +
                    (state.is_playing ? "Playing" : "Paused")),
               text("Progress: " + BuildProgressBar(ratio, 40)),
               text("Time: " + std::to_string(state.elapsed_sec) + " / " +
                    std::to_string(state.duration_sec) + " sec"),
               separator(),
               text("Spectrum"),
               RenderSpectrum(latest_frame.spectrum_bars),
               separator(),
               text("Waveform"),
               RenderWaveform(latest_frame.waveform_points),
               separator(),
               text("Playlist (h/l prev/next, Space pause/resume, j/k move, "
                    "click select, Enter play)"),
               RenderSelectionStatus(latest_playlist),
               RenderPlaylist(latest_playlist, view_offset.load(),
                              kPlaylistVisibleRows),
               separator(),
               text("Press q to quit."),
           }) |
           border;
  });
  component |= CatchEvent([&](Event event) {
    if (event == Event::Character(keybindings.quit)) {
      on_intent(UiIntent::kQuit);
      keep_running.store(false);
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Character(keybindings.previous_track)) {
      on_intent(UiIntent::kPreviousTrack);
      keep_running.store(false);
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Character(keybindings.next_track)) {
      on_intent(UiIntent::kNextTrack);
      keep_running.store(false);
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Character(keybindings.toggle_pause)) {
      on_intent(UiIntent::kTogglePause);
      return true;
    }
    if (event == Event::Character(keybindings.move_selection_up)) {
      select_delta(-1);
      return true;
    }
    if (event == Event::Character(keybindings.move_selection_down)) {
      select_delta(1);
      return true;
    }
    if (event == Event::Return) {
      play_selected();
      return true;
    }
    if (event.is_mouse()) {
      const Mouse& mouse = event.mouse();
      if (mouse.button == Mouse::WheelUp) {
        int next_offset =
            ClampOffset(view_offset.load() - 1,
                        static_cast<int>(latest_playlist.tracks.size()),
                        kPlaylistVisibleRows);
        view_offset.store(next_offset);
        return true;
      }
      if (mouse.button == Mouse::WheelDown) {
        int next_offset =
            ClampOffset(view_offset.load() + 1,
                        static_cast<int>(latest_playlist.tracks.size()),
                        kPlaylistVisibleRows);
        view_offset.store(next_offset);
        return true;
      }
      if (mouse.button == Mouse::Left && mouse.motion == Mouse::Released) {
        int local_row = mouse.y - kPlaylistStartY;
        int track_count = static_cast<int>(latest_playlist.tracks.size());
        int offset = view_offset.load();
        int visible_count =
            std::min(kPlaylistVisibleRows, std::max(0, track_count - offset));
        int leading_ellipsis_rows = offset > 0 ? 1 : 0;
        int row_in_tracks = local_row - leading_ellipsis_rows;
        if (row_in_tracks >= 0 && row_in_tracks < visible_count) {
          int clicked_index = offset + row_in_tracks;
          selected_index.store(clicked_index);
          int next_offset =
              FollowSelectedOffset(clicked_index, offset, kPlaylistVisibleRows);
          next_offset =
              ClampOffset(next_offset, track_count, kPlaylistVisibleRows);
          view_offset.store(next_offset);
          on_selection_changed(clicked_index);
          return true;
        }
      }
    }
    return false;
  });

  std::thread refresh_thread([&] {
    while (keep_running.load()) {
      latest_frame = frame_provider();
      int previous_selected = selected_index.load();
      latest_playlist = playlist_provider();
      selected_index.store(latest_playlist.selected_track_index);
      int current_selected = selected_index.load();
      if (current_selected != previous_selected) {
        int next_offset = FollowSelectedOffset(
            current_selected, view_offset.load(), kPlaylistVisibleRows);
        next_offset = ClampOffset(
            next_offset, static_cast<int>(latest_playlist.tracks.size()),
            kPlaylistVisibleRows);
        view_offset.store(next_offset);
      } else {
        int next_offset = ClampOffset(
            view_offset.load(), static_cast<int>(latest_playlist.tracks.size()),
            kPlaylistVisibleRows);
        view_offset.store(next_offset);
      }
      if (should_stop()) {
        keep_running.store(false);
        screen.ExitLoopClosure()();
        return;
      }
      screen.PostEvent(Event::Custom);
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
  });

  screen.Loop(component);
  keep_running.store(false);
  if (refresh_thread.joinable()) {
    refresh_thread.join();
  }
}

// Build textual progress bar from playback ratio.
std::string TuiRenderer::BuildProgressBar(float ratio, int width) {
  ratio = std::clamp(ratio, 0.0f, 1.0f);
  int filled = static_cast<int>(ratio * static_cast<float>(width));
  return "[" + std::string(filled, '=') + std::string(width - filled, ' ') +
         "]";
}

}  // namespace vocalplayer
