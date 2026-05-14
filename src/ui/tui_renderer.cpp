/**
 * @file tui_renderer.cpp
 * @brief Implements terminal rendering and user input dispatch with FTXUI.
 *
 * Key points:
 * - Builds spectrum, waveform, progress, and playlist visual blocks.
 * - Tracks playlist viewport and selection-follow behavior.
 * - Converts keyboard/mouse events into high-level UiIntent callbacks.
 * - Drives VisualUpdatePipeline for off-thread analysis and coalesced redraw
 *   wakeups (avoids flooding FTXUI with Event::Custom).
 */
#include "ui/tui_renderer.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/mouse.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ui/theme.hpp"
#include "ui/visual_update_pipeline.hpp"

namespace vocalplayer {
namespace {

using namespace ftxui;  // NOLINT

constexpr int kPlaylistVisibleRows = 10;
constexpr int kSpectrumCanvasHeight = 32;
constexpr int kWaveformCanvasHeight = 20;
constexpr int kStereoMeterGaugeWidth = 14;
constexpr float kSpectrumPeakThreshold = 0.03f;

// Format seconds as mm:ss for compact top status line.
std::string FormatTime(double total_seconds) {
  const int rounded = std::max(0, static_cast<int>(std::round(total_seconds)));
  const int minutes = rounded / 60;
  const int seconds = rounded % 60;
  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setw(2)
      << seconds;
  return oss.str();
}

// Render normalized spectrum bars and peak-hold markers.
Element RenderSpectrum(const std::vector<float>& bars,
                       const std::vector<float>& peaks, const Theme& theme) {
  if (bars.empty()) {
    return text("(no spectrum data)");
  }
  const int canvas_width = static_cast<int>(bars.size() * 2);
  return canvas(
      canvas_width, kSpectrumCanvasHeight,
      [bars, peaks, theme, canvas_width](Canvas& c) {
        const int baseline = kSpectrumCanvasHeight - 1;
        for (size_t idx = 0; idx < bars.size(); ++idx) {
          const int bar_x = static_cast<int>(idx * 2);
          const float value = std::clamp(bars[idx], 0.0F, 1.0F);
          const float peak_value =
              idx < peaks.size() ? std::clamp(peaks[idx], 0.0F, 1.0F) : value;
          const int filled_height = static_cast<int>(
              value * static_cast<float>(kSpectrumCanvasHeight - 1));
          const int bar_top = baseline - filled_height;
          c.DrawPointLine(bar_x, baseline, bar_x, bar_top,
                          theme.spectrum_color);
          if (bar_x + 1 < canvas_width) {
            c.DrawPointLine(bar_x + 1, baseline, bar_x + 1, bar_top,
                            theme.spectrum_color);
          }

          const bool should_draw_peak =
              peak_value > value + kSpectrumPeakThreshold && peak_value > 0.0F;
          if (should_draw_peak) {
            const int peak_y =
                baseline -
                static_cast<int>(peak_value *
                                 static_cast<float>(kSpectrumCanvasHeight - 1));
            c.DrawPoint(bar_x, peak_y, true, theme.peak_color);
            if (bar_x + 1 < canvas_width) {
              c.DrawPoint(bar_x + 1, peak_y, true, theme.peak_color);
            }
          }
        }
      });
}

// Render waveform points as a connected canvas line graph.
Element RenderWaveform(const std::vector<float>& wave, const Theme& theme) {
  if (wave.empty()) {
    return text("(no waveform data)");
  }
  const int canvas_width = static_cast<int>(wave.size());
  return canvas(
      canvas_width, kWaveformCanvasHeight,
      [wave, theme, canvas_width](Canvas& c) {
        const int max_y = kWaveformCanvasHeight - 1;
        int previous_x = 0;
        int previous_y = max_y;
        for (size_t idx = 0; idx < wave.size(); ++idx) {
          const int current_x = static_cast<int>(idx);
          const float value = std::clamp(wave[idx], 0.0F, 1.0F);
          const int current_y =
              static_cast<int>((1.0F - value) * static_cast<float>(max_y));
          if (idx == 0) {
            c.DrawPoint(current_x, current_y, true, theme.waveform_color);
          } else {
            c.DrawPointLine(previous_x, previous_y, current_x, current_y,
                            theme.waveform_color);
          }
          previous_x = current_x;
          previous_y = current_y;
        }
        if (canvas_width > 0) {
          c.DrawPointLine(0, max_y, canvas_width - 1, max_y, Color::GrayDark);
        }
      });
}

/**
 * @brief Render RMS/Peak/Band gauges for one analyzed channel.
 *
 * @param title Window title text.
 * @param ch Per-channel visualization snapshot.
 * @param theme Active color theme.
 * @param gauge_width Fixed gauge width in terminal cells.
 * @return FTXUI element tree for the meters panel.
 */
Element RenderChannelMetersWindow(const std::string& title,
                                  const ChannelVisuals& ch, const Theme& theme,
                                  int gauge_width) {
  return window(
      text(title) | color(theme.title_color),
      vbox({
          hbox({text("RMS  ") | color(theme.text_color),
                gauge(ch.rms_level) | color(theme.meter_color) |
                    size(WIDTH, EQUAL, gauge_width)}),
          hbox({text("Peak ") | color(theme.text_color),
                gauge(ch.peak_level) | color(theme.warning_color) |
                    size(WIDTH, EQUAL, gauge_width)}),
          separator(),
          text("Bands") | color(theme.accent_color),
          hbox(
              {text("Low  ") | color(theme.text_color),
               gauge(ch.band_energies.size() > 0 ? ch.band_energies[0] : 0.0F) |
                   color(theme.spectrum_color) |
                   size(WIDTH, EQUAL, gauge_width)}),
          hbox(
              {text("Mid  ") | color(theme.text_color),
               gauge(ch.band_energies.size() > 1 ? ch.band_energies[1] : 0.0F) |
                   color(theme.accent_color) |
                   size(WIDTH, EQUAL, gauge_width)}),
          hbox(
              {text("High ") | color(theme.text_color),
               gauge(ch.band_energies.size() > 2 ? ch.band_energies[2] : 0.0F) |
                   color(theme.peak_color) | size(WIDTH, EQUAL, gauge_width)}),
      }));
}

/**
 * @brief Lay out left/right spectrum canvases horizontally.
 *
 * @param left Left channel visuals.
 * @param right Right channel visuals.
 * @param theme Active color theme.
 * @return FTXUI element tree for the stereo spectrum row.
 */
Element StereoSpectrumRow(const ChannelVisuals& left,
                          const ChannelVisuals& right, const Theme& theme) {
  return hbox({
      window(
          text("Spectrum L") | color(theme.title_color),
          RenderSpectrum(left.spectrum_bars, left.spectrum_peak_bars, theme)) |
          flex,
      window(text("Spectrum R") | color(theme.title_color),
             RenderSpectrum(right.spectrum_bars, right.spectrum_peak_bars,
                            theme)) |
          flex,
  });
}

/**
 * @brief Lay out left/right waveform canvases horizontally.
 *
 * @param left Left channel visuals.
 * @param right Right channel visuals.
 * @param use_envelope When true, draw envelope waveform points per side.
 * @param theme Active color theme.
 * @return FTXUI element tree for the stereo waveform row.
 */
Element StereoWaveformRow(const ChannelVisuals& left,
                          const ChannelVisuals& right, bool use_envelope,
                          const Theme& theme) {
  const std::vector<float>& wave_l =
      use_envelope ? left.waveform_envelope_points : left.waveform_points;
  const std::vector<float>& wave_r =
      use_envelope ? right.waveform_envelope_points : right.waveform_points;
  const std::string style = use_envelope ? "(Env)" : "(Raw)";
  return hbox({
      window(text("Wave L " + style) | color(theme.title_color),
             RenderWaveform(wave_l, theme)) |
          flex,
      window(text("Wave R " + style) | color(theme.title_color),
             RenderWaveform(wave_r, theme)) |
          flex,
  });
}

/**
 * @brief Lay out left/right meter panels horizontally.
 *
 * @param left Left channel visuals.
 * @param right Right channel visuals.
 * @param theme Active color theme.
 * @param gauge_width Fixed gauge width in terminal cells.
 * @return FTXUI element tree for the stereo meters row.
 */
Element StereoMetersRow(const ChannelVisuals& left, const ChannelVisuals& right,
                        const Theme& theme, int gauge_width) {
  return hbox({
      RenderChannelMetersWindow("Meters L", left, theme, gauge_width) | flex,
      RenderChannelMetersWindow("Meters R", right, theme, gauge_width) | flex,
  });
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
Element RenderSelectionStatus(const PlaylistViewModel& playlist,
                              const Theme& theme) {
  if (playlist.tracks.empty()) {
    return text("Selected: none (playlist is empty)") |
           color(theme.warning_color);
  }

  const bool is_pending =
      playlist.selected_track_index != playlist.current_track_index;
  const std::string status_label = is_pending ? "PENDING" : "LIVE";
  const Color status_color =
      is_pending ? theme.warning_color : theme.meter_color;
  const std::string selected_text =
      std::to_string(playlist.selected_track_index + 1) + "/" +
      std::to_string(playlist.tracks.size());

  return hbox({
      text("Selected "),
      text(selected_text) | bold | color(theme.accent_color),
      text("  "),
      text("[" + status_label + "]") | bold | color(status_color),
      text("  "),
      text("(Press Enter to play)"),
  });
}

// Cycle layout mode for the main visualization region.
VisualMode NextVisualMode(VisualMode mode) {
  switch (mode) {
    case VisualMode::kOverview:
      return VisualMode::kSpectrumFocus;
    case VisualMode::kSpectrumFocus:
      return VisualMode::kWaveformFocus;
    case VisualMode::kWaveformFocus:
      return VisualMode::kMeterFocus;
    case VisualMode::kMeterFocus:
    default:
      return VisualMode::kOverview;
  }
}

// Build a concise mode label shown in footer/status.
std::string VisualModeName(VisualMode mode) {
  switch (mode) {
    case VisualMode::kSpectrumFocus:
      return "Spectrum";
    case VisualMode::kWaveformFocus:
      return "Waveform";
    case VisualMode::kMeterFocus:
      return "Meters";
    case VisualMode::kOverview:
    default:
      return "Overview";
  }
}

// Detect whether a mouse event occurred inside the given box.
bool IsInsideBox(const Box& box, int x, int y) {
  return x >= box.x_min && x <= box.x_max && y >= box.y_min && y <= box.y_max;
}

}  // namespace

// Run one interactive TUI session and map raw input to playback intents.
void TuiRenderer::Run(
    const std::function<VisualFrame()>& frame_provider,
    const std::function<PlaylistViewModel()>& playlist_provider,
    const std::function<void(UiIntent)>& on_intent,
    const std::function<void(int)>& on_selection_changed,
    const std::function<bool()>& should_stop, UiSessionState* session_state) {
  PlaylistViewModel latest_playlist = playlist_provider();
  Keybindings keybindings = DefaultKeybindings();
  ThemeId active_theme_id = ThemeId::kDefault;
  VisualMode active_visual_mode = VisualMode::kOverview;
  bool use_envelope_waveform = false;
  if (session_state != nullptr) {
    active_theme_id = session_state->theme_id;
    active_visual_mode = session_state->visual_mode;
    use_envelope_waveform = session_state->use_envelope_waveform;
  }

  auto screen = ScreenInteractive::Fullscreen();
  VisualUpdatePipeline visual_pipeline(&screen, frame_provider, should_stop,
                                       std::chrono::milliseconds(33),
                                       std::chrono::milliseconds(50));
  visual_pipeline.Prime();
  VisualFrame latest_frame;
  visual_pipeline.CopyLatestFrame(&latest_frame);
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
  Box playlist_rows_box;

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
    screen.ExitLoopClosure()();
  };

  auto component = Renderer([&] {
    if (!visual_pipeline.IsStarted()) {
      visual_pipeline.Start();
    }
    visual_pipeline.CopyLatestFrame(&latest_frame);
    int previous_selected = selected_index.load();
    latest_playlist = playlist_provider();
    selected_index.store(latest_playlist.selected_track_index);
    int current_selected = selected_index.load();
    if (current_selected != previous_selected) {
      int next_offset = FollowSelectedOffset(
          current_selected, view_offset.load(), kPlaylistVisibleRows);
      next_offset = ClampOffset(next_offset,
                                static_cast<int>(latest_playlist.tracks.size()),
                                kPlaylistVisibleRows);
      view_offset.store(next_offset);
    } else {
      int next_offset = ClampOffset(
          view_offset.load(), static_cast<int>(latest_playlist.tracks.size()),
          kPlaylistVisibleRows);
      view_offset.store(next_offset);
    }

    const Theme& theme = GetBuiltinTheme(active_theme_id);
    const PlaybackState& state = latest_frame.playback_state;
    float ratio = 0.0f;
    if (state.duration_sec > 0.0) {
      ratio = static_cast<float>(state.elapsed_sec / state.duration_sec);
    }

    const ChannelVisuals& ch_l = latest_frame.left;
    const ChannelVisuals& ch_r = latest_frame.right;
    Element stereo_spectrum = StereoSpectrumRow(ch_l, ch_r, theme);
    Element stereo_waveform =
        StereoWaveformRow(ch_l, ch_r, use_envelope_waveform, theme);
    Element stereo_meters =
        StereoMetersRow(ch_l, ch_r, theme, kStereoMeterGaugeWidth);

    Element visual_area;
    switch (active_visual_mode) {
      case VisualMode::kSpectrumFocus:
        visual_area = hbox({stereo_spectrum | flex, stereo_meters | flex});
        break;
      case VisualMode::kWaveformFocus:
        visual_area = hbox({stereo_waveform | flex, stereo_meters | flex});
        break;
      case VisualMode::kMeterFocus:
        visual_area = hbox({stereo_meters | flex, stereo_spectrum | flex});
        break;
      case VisualMode::kOverview:
      default:
        visual_area = vbox({
            stereo_spectrum | flex,
            stereo_waveform | flex,
            stereo_meters | flex,
        });
        break;
    }

    Element playlist_rows = RenderPlaylist(latest_playlist, view_offset.load(),
                                           kPlaylistVisibleRows) |
                            reflect(playlist_rows_box);
    Element playlist_panel =
        window(text("Playlist") | color(theme.title_color),
               vbox({
                   text("h/l prev/next  Space pause  j/k select  Enter play") |
                       color(theme.text_color),
                   RenderSelectionStatus(latest_playlist, theme),
                   separator(),
                   playlist_rows,
               }));

    Element header_panel =
        window(hcenter(text("Now Playing") | color(theme.title_color)),
               vbox({
                   hcenter(text(latest_frame.track_info.title) |
                           color(theme.text_color)),
                   hcenter(text(latest_frame.track_info.artist) |
                           color(theme.text_color)),
                   hcenter(text(state.is_playing ? "Playing" : "Paused") |
                           color(theme.accent_color)),
                   hcenter(text(BuildProgressBar(ratio, 36)) |
                           color(theme.text_color)),
                   hcenter(text(FormatTime(state.elapsed_sec) + " / " +
                                FormatTime(state.duration_sec)) |
                           color(theme.text_color)),
               }));

    Element footer =
        text("Mode[m]: " + VisualModeName(active_visual_mode) + " | Wave[v]: " +
             std::string(use_envelope_waveform ? "Envelope" : "Raw") +
             " | Theme[t]: " + GetThemeDisplayName(active_theme_id) +
             " | q quit") |
        color(theme.accent_color);

    return vbox({
               hcenter(text("vocalplayer") | bold | color(theme.title_color)),
               header_panel,
               visual_area,
               playlist_panel,
               footer,
           }) |
           border | color(theme.border_color);
  });
  component |= CatchEvent([&](Event event) {
    if (event == Event::Character(keybindings.quit)) {
      on_intent(UiIntent::kQuit);
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Character(keybindings.previous_track)) {
      on_intent(UiIntent::kPreviousTrack);
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Character(keybindings.next_track)) {
      on_intent(UiIntent::kNextTrack);
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
    if (event == Event::Character(keybindings.cycle_visual_mode)) {
      active_visual_mode = NextVisualMode(active_visual_mode);
      return true;
    }
    if (event == Event::Character(keybindings.toggle_waveform_style)) {
      use_envelope_waveform = !use_envelope_waveform;
      return true;
    }
    if (event == Event::Character(keybindings.cycle_theme)) {
      active_theme_id = NextThemeId(active_theme_id);
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
        if (!IsInsideBox(playlist_rows_box, mouse.x, mouse.y)) {
          return false;
        }
        int local_row = mouse.y - playlist_rows_box.y_min;
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

  screen.Loop(component);
  visual_pipeline.Stop();
  if (session_state != nullptr) {
    session_state->theme_id = active_theme_id;
    session_state->visual_mode = active_visual_mode;
    session_state->use_envelope_waveform = use_envelope_waveform;
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
