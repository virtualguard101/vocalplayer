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

std::string BuildBar(float value, int height) {
  int filled = static_cast<int>(value * static_cast<float>(height));
  return std::string(std::max(filled, 0), '#');
}

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

Element RenderPlaylist(const PlaylistViewModel& playlist) {
  std::vector<Element> rows;
  rows.reserve(playlist.tracks.size());
  for (int i = 0; i < static_cast<int>(playlist.tracks.size()); ++i) {
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
  if (rows.empty()) {
    rows.push_back(text("(empty playlist)"));
  }
  return vbox(std::move(rows)) | yframe | size(HEIGHT, LESS_THAN, 10);
}

}  // namespace

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
  std::atomic<int> selected_index{latest_playlist.selected_track_index};

  auto select_delta = [&](int delta) {
    if (latest_playlist.tracks.empty()) {
      return;
    }
    int current_selected = selected_index.load();
    int next_selected =
        std::clamp(current_selected + delta, 0,
                   static_cast<int>(latest_playlist.tracks.size()) - 1);
    selected_index.store(next_selected);
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
                    "Enter/click play)"),
               RenderPlaylist(latest_playlist),
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
      if (mouse.button == Mouse::Left && mouse.motion == Mouse::Released) {
        const int playlist_start_y = 16;
        int clicked_index = mouse.y - playlist_start_y;
        if (clicked_index >= 0 &&
            clicked_index < static_cast<int>(latest_playlist.tracks.size())) {
          selected_index.store(clicked_index);
          on_selection_changed(clicked_index);
          play_selected();
          return true;
        }
      }
    }
    return false;
  });

  std::thread refresh_thread([&] {
    while (keep_running.load()) {
      latest_frame = frame_provider();
      latest_playlist = playlist_provider();
      selected_index.store(latest_playlist.selected_track_index);
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

std::string TuiRenderer::BuildProgressBar(float ratio, int width) {
  ratio = std::clamp(ratio, 0.0f, 1.0f);
  int filled = static_cast<int>(ratio * static_cast<float>(width));
  return "[" + std::string(filled, '=') + std::string(width - filled, ' ') +
         "]";
}

}  // namespace vocalplayer
