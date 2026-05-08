#ifndef VOCALPLAYER_SRC_ANALYSIS_SPECTRUM_ANALYZER_HPP_
#define VOCALPLAYER_SRC_ANALYSIS_SPECTRUM_ANALYZER_HPP_

#include <cstdint>
#include <vector>

namespace vocalplayer {

class SpectrumAnalyzer {
 public:
  SpectrumAnalyzer(uint32_t fft_size, uint32_t bar_count, float smooth_factor);
  ~SpectrumAnalyzer();

  SpectrumAnalyzer(const SpectrumAnalyzer&) = delete;
  SpectrumAnalyzer& operator=(const SpectrumAnalyzer&) = delete;

  std::vector<float> ComputeBars(const std::vector<float>& mono_window);
  std::vector<float> ComputeWaveform(const std::vector<float>& mono_window,
                                     uint32_t points) const;

 private:
  uint32_t fft_size_;
  uint32_t bar_count_;
  float smooth_factor_;
  std::vector<float> smoothed_bars_;

  struct Impl;
  Impl* impl_;
};

}  // namespace vocalplayer

#endif  // VOCALPLAYER_SRC_ANALYSIS_SPECTRUM_ANALYZER_HPP_
