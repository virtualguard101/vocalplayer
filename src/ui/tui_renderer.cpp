#include "ui/tui_renderer.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "ftxui/component/component.hpp"
#include "ftxui/component/event.hpp"
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

}  // namespace

void TuiRenderer::Run(const std::function<VisualFrame()>& frame_provider,
                      const std::function<bool()>& should_stop) {
  VisualFrame latest_frame = frame_provider();
  std::atomic<bool> keep_running = true;

  auto screen = ScreenInteractive::Fullscreen();
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
               text("Press q to quit."),
           }) |
           border;
  });

  component |= CatchEvent([&](Event event) {
    if (event == Event::Character('q') || should_stop()) {
      keep_running.store(false);
      screen.ExitLoopClosure()();
      return true;
    }
    return false;
  });

  std::thread refresh_thread([&] {
    while (keep_running.load()) {
      latest_frame = frame_provider();
      if (should_stop()) {
        keep_running.store(false);
        screen.PostEvent(Event::Character('q'));
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
