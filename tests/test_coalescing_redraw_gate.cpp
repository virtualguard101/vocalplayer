/**
 * @file test_coalescing_redraw_gate.cpp
 * @brief Unit tests for CoalescingRedrawGate coalescing semantics.
 */

#include <atomic>
#include <cassert>

#include "ui/coalescing_redraw_gate.hpp"

int main() {
  std::atomic<int> flush_count{0};
  vocalplayer::CoalescingRedrawGate gate([&]() { flush_count.fetch_add(1); });

  for (int i = 0; i < 100; ++i) {
    gate.RequestFlush();
  }
  assert(flush_count.load() == 1);
  assert(gate.Posted());

  gate.MarkFlushed();
  assert(!gate.Posted());

  gate.RequestFlush();
  assert(flush_count.load() == 2);

  return 0;
}
