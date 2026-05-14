/**
 * @file visual_update_pipeline.hpp
 * @brief Worker-driven visual frame production with coalesced FTXUI redraw
 * wakeups.
 *
 * Key points:
 * - Runs heavy frame_provider work off the FTXUI main loop.
 * - Publishes snapshots under a mutex for the renderer to copy each draw.
 * - Uses Post(Closure)+RequestAnimationFrame instead of PostEvent(Custom)
 * storms.
 */
#ifndef VOCALPLAYER_SRC_UI_VISUAL_UPDATE_PIPELINE_HPP_
#define VOCALPLAYER_SRC_UI_VISUAL_UPDATE_PIPELINE_HPP_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "ftxui/component/screen_interactive.hpp"
#include "shared/types.hpp"
#include "ui/coalescing_redraw_gate.hpp"

namespace vocalplayer {

/**
 * @brief Owns the background visual refresh thread and thread-safe frame
 * publish.
 */
class VisualUpdatePipeline {
 public:
  /**
   * @brief Construct a pipeline bound to a fullscreen screen instance.
   *
   * @param screen Non-null FTXUI screen receiving coalesced redraw requests.
   * @param produce_frame Builds one VisualFrame snapshot (may be heavy).
   * @param should_stop When true, worker stops and triggers ExitLoopClosure.
   * @param tick Target interval between production starts.
   * @param slow_work_ms If produce_frame exceeds this, extra backoff is
   * applied.
   *
   * @return Nothing.
   */
  VisualUpdatePipeline(ftxui::ScreenInteractive* screen,
                       std::function<VisualFrame()> produce_frame,
                       std::function<bool()> should_stop,
                       std::chrono::milliseconds tick,
                       std::chrono::milliseconds slow_work_ms);

  VisualUpdatePipeline(const VisualUpdatePipeline&) = delete;
  VisualUpdatePipeline& operator=(const VisualUpdatePipeline&) = delete;

  /**
   * @brief Synchronously produce and publish one frame before the worker runs.
   *
   * @return Nothing.
   */
  void Prime();

  /**
   * @brief Start the background worker thread.
   *
   * @note Must run only after FTXUI has installed the task sender (e.g. first
   *       Renderer tick inside ScreenInteractive::Loop); otherwise Post() is a
   *       no-op.
   *
   * @return Nothing.
   */
  void Start();

  /**
   * @brief True after Start() has been called.
   *
   * @return Whether the worker thread was started.
   */
  [[nodiscard]] bool IsStarted() const { return worker_started_.load(); }

  /**
   * @brief Request worker exit and join the thread.
   *
   * @return Nothing.
   */
  void Stop();

  /**
   * @brief Copy the latest published frame for rendering.
   *
   * @param out Destination snapshot (overwritten).
   *
   * @return Nothing.
   */
  void CopyLatestFrame(VisualFrame* out) const;

  /**
   * @brief Joins the worker thread if still running.
   *
   * @return Nothing.
   */
  ~VisualUpdatePipeline();

 private:
  /**
   * @brief Background loop: produce, publish, schedule coalesced redraw, sleep.
   */
  void WorkerLoop();
  /**
   * @brief Arm at most one pending FTXUI wakeup until the main thread flushes.
   */
  void ScheduleRedrawCoalesced();

  ftxui::ScreenInteractive* screen_;
  std::function<VisualFrame()> produce_frame_;
  std::function<bool()> should_stop_;
  std::chrono::milliseconds tick_;
  std::chrono::milliseconds slow_work_ms_;

  mutable std::mutex frame_mutex_;
  VisualFrame published_frame_;

  std::atomic<bool> stop_requested_{false};
  std::atomic<bool> worker_started_{false};
  mutable std::mutex cv_mutex_;
  std::condition_variable cv_;
  std::thread worker_;

  CoalescingRedrawGate redraw_gate_;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_UI_VISUAL_UPDATE_PIPELINE_HPP_
