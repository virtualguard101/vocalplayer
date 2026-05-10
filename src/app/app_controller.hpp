/**
 * @file app_controller.hpp
 * @brief Declares AppController, the high-level player workflow coordinator.
 *
 * Key points:
 * - Defines Run() as the main application control entry.
 * - Aggregates decoding, metadata, playback, analysis, and UI modules.
 * - Encapsulates runtime state machine ownership in one class.
 */
#ifndef VOCALPLAYER_SRC_APP_APP_CONTROLLER_HPP_
#define VOCALPLAYER_SRC_APP_APP_CONTROLLER_HPP_

#include <string>
#include <vector>

#include "analysis/spectrum_analyzer.hpp"
#include "app/playlist.hpp"
#include "audio/audio_engine.hpp"
#include "audio/decoder.hpp"
#include "audio/metadata.hpp"
#include "ui/tui_renderer.hpp"

namespace vocalplayer {

/**
 * @brief High-level application orchestrator for playlist playback.
 *
 * AppController wires decoding, metadata loading, audio playback, analysis,
 * and TUI rendering into one runtime loop.
 */
class AppController {
 public:
  /**
   * @brief Start the player with a file path or directory path.
   *
   * @param input_path Audio file path or directory path for playlist scan.
   * @return Process-style exit code. 0 means success.
   *
   * @note This function owns the main playback state machine.
   */
  int Run(const std::string& input_path);

 private:
  Decoder decoder_;
  MetadataReader metadata_reader_;
  AudioEngine audio_engine_;
  SpectrumAnalyzer analyzer_{2048, 48, 0.85f};
  TuiRenderer tui_renderer_;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_APP_APP_CONTROLLER_HPP_
