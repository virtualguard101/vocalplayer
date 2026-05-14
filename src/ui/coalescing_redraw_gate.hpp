/**
 * @file coalescing_redraw_gate.hpp
 * @brief Coalesces many producer-side refresh signals into one consumer
 * callback.
 *
 * Key points:
 * - Used by VisualUpdatePipeline to avoid flooding FTXUI with redundant
 * wakeups.
 * - Thread-safe for one producer calling RequestFlush concurrently with
 * MarkFlushed.
 */
#ifndef VOCALPLAYER_SRC_UI_COALESCING_REDRAW_GATE_HPP_
#define VOCALPLAYER_SRC_UI_COALESCING_REDRAW_GATE_HPP_

#include <atomic>
#include <functional>

namespace vocalplayer {

/**
 * @brief Arm at most one pending flush until MarkFlushed clears the arm bit.
 */
class CoalescingRedrawGate {
 public:
  /**
   * @brief Construct a gate with a callback invoked when a new arm succeeds.
   *
   * @param on_flush Invoked synchronously from RequestFlush when transitioning
   *        from unarmed to armed (typically schedules work on the UI thread).
   *
   * @return Nothing.
   */
  explicit CoalescingRedrawGate(std::function<void()> on_flush)
      : on_flush_(std::move(on_flush)) {}

  /**
   * @brief Request a flush; runs on_flush at most once until MarkFlushed.
   *
   * @return Nothing.
   */
  void RequestFlush() {
    if (!posted_.exchange(true)) {
      if (on_flush_) {
        on_flush_();
      }
    }
  }

  /**
   * @brief Clear the armed state after the flush has been applied.
   *
   * @return Nothing.
   */
  void MarkFlushed() { posted_.store(false); }

  /**
   * @brief Query whether a flush is still considered pending.
   *
   * @return True when posted_ is set.
   */
  [[nodiscard]] bool Posted() const { return posted_.load(); }

 private:
  std::atomic<bool> posted_{false};
  std::function<void()> on_flush_;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_UI_COALESCING_REDRAW_GATE_HPP_
