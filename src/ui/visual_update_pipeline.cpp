/**
 * @file visual_update_pipeline.cpp
 * @brief Implements VisualUpdatePipeline worker loop and coalesced redraw
 * posts.
 */

#include "ui/visual_update_pipeline.hpp"

#include <utility>

namespace vocalplayer {

VisualUpdatePipeline::VisualUpdatePipeline(
    ftxui::ScreenInteractive* screen,
    std::function<VisualFrame()> produce_frame,
    std::function<bool()> should_stop, std::chrono::milliseconds tick,
    std::chrono::milliseconds slow_work_ms)
    : screen_(screen),
      produce_frame_(std::move(produce_frame)),
      should_stop_(std::move(should_stop)),
      tick_(tick),
      slow_work_ms_(slow_work_ms),
      redraw_gate_([this]() {
        if (screen_ == nullptr) {
          return;
        }
        screen_->Post([this]() {
          redraw_gate_.MarkFlushed();
          screen_->RequestAnimationFrame();
        });
      }) {}

VisualUpdatePipeline::~VisualUpdatePipeline() { Stop(); }

void VisualUpdatePipeline::Prime() {
  VisualFrame frame = produce_frame_();
  std::lock_guard<std::mutex> lock(frame_mutex_);
  published_frame_ = std::move(frame);
}

void VisualUpdatePipeline::Start() {
  if (worker_started_.exchange(true)) {
    return;
  }
  stop_requested_.store(false);
  worker_ = std::thread([this] { WorkerLoop(); });
}

void VisualUpdatePipeline::Stop() {
  stop_requested_.store(true);
  cv_.notify_all();
  if (worker_.joinable()) {
    worker_.join();
  }
  worker_started_.store(false);
}

void VisualUpdatePipeline::CopyLatestFrame(VisualFrame* out) const {
  std::lock_guard<std::mutex> lock(frame_mutex_);
  *out = published_frame_;
}

void VisualUpdatePipeline::ScheduleRedrawCoalesced() {
  redraw_gate_.RequestFlush();
}

void VisualUpdatePipeline::WorkerLoop() {
  using clock = std::chrono::steady_clock;
  while (!stop_requested_.load()) {
    const clock::time_point t0 = clock::now();
    VisualFrame frame = produce_frame_();
    {
      std::lock_guard<std::mutex> lock(frame_mutex_);
      published_frame_ = std::move(frame);
    }
    ScheduleRedrawCoalesced();

    if (should_stop_()) {
      stop_requested_.store(true);
      if (screen_ != nullptr) {
        screen_->ExitLoopClosure()();
      }
      break;
    }

    const clock::time_point t1 = clock::now();
    const auto work_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
    int sleep_ms = static_cast<int>(tick_.count() - work_ms.count());
    if (work_ms > slow_work_ms_) {
      sleep_ms += static_cast<int>(work_ms.count() / 2);
    }
    if (sleep_ms < 1) {
      sleep_ms = 1;
    }
    std::unique_lock<std::mutex> lk(cv_mutex_);
    cv_.wait_for(lk, std::chrono::milliseconds(sleep_ms),
                 [this] { return stop_requested_.load(); });
  }
}

}  // namespace vocalplayer
