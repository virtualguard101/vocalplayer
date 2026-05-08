#include "analysis/spectrum_analyzer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include "kiss_fft.h"

namespace vocalplayer {
namespace {

constexpr float kPi = 3.14159265358979323846f;

// Clamp float into [0, 1] range.
float Clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

}  // namespace

struct SpectrumAnalyzer::Impl {
  // Allocate kissfft config and buffers for one analyzer instance.
  explicit Impl(int fft_size)
      : cfg(kiss_fft_alloc(fft_size, 0, nullptr, nullptr)),
        input(fft_size),
        output(fft_size) {}

  // Release FFT plan allocated by kissfft.
  ~Impl() { free(cfg); }

  kiss_fft_cfg cfg = nullptr;
  std::vector<kiss_fft_cpx> input;
  std::vector<kiss_fft_cpx> output;
};

// Construct FFT analyzer state and validate runtime parameters.
SpectrumAnalyzer::SpectrumAnalyzer(uint32_t fft_size, uint32_t bar_count,
                                   float smooth_factor)
    : fft_size_(fft_size),
      bar_count_(bar_count),
      smooth_factor_(smooth_factor),
      smoothed_bars_(bar_count, 0.0f),
      impl_(new Impl(static_cast<int>(fft_size))) {
  if (fft_size_ == 0 || (fft_size_ % 2) != 0) {
    throw std::invalid_argument("fft_size must be a non-zero even number.");
  }
  if (bar_count_ == 0) {
    throw std::invalid_argument("bar_count must be non-zero.");
  }
  if (impl_->cfg == nullptr) {
    throw std::runtime_error("kiss_fft_alloc failed.");
  }
}

// Release opaque FFT implementation state.
SpectrumAnalyzer::~SpectrumAnalyzer() { delete impl_; }

// Compute smoothed log-compressed spectrum bars from mono window.
std::vector<float> SpectrumAnalyzer::ComputeBars(
    const std::vector<float>& mono_window) {
  std::vector<float> padded(fft_size_, 0.0f);
  if (!mono_window.empty()) {
    uint32_t copy_count = std::min<uint32_t>(mono_window.size(), fft_size_);
    std::copy_n(mono_window.end() - copy_count, copy_count,
                padded.end() - copy_count);
  }

  for (uint32_t i = 0; i < fft_size_; ++i) {
    float hann = 0.5f * (1.0f - std::cos((2.0f * kPi * i) / (fft_size_ - 1)));
    impl_->input[i].r = padded[i] * hann;
    impl_->input[i].i = 0.0f;
  }

  kiss_fft(impl_->cfg, impl_->input.data(), impl_->output.data());

  uint32_t nyquist_bins = fft_size_ / 2;
  std::vector<float> bars(bar_count_, 0.0f);
  for (uint32_t bar = 0; bar < bar_count_; ++bar) {
    uint32_t start = (bar * nyquist_bins) / bar_count_;
    uint32_t end = ((bar + 1) * nyquist_bins) / bar_count_;
    end = std::max(end, start + 1);

    float energy = 0.0f;
    for (uint32_t bin = start; bin < end; ++bin) {
      float real = impl_->output[bin].r;
      float imag = impl_->output[bin].i;
      energy += std::sqrt(real * real + imag * imag);
    }
    energy /= static_cast<float>(end - start);

    // Log compression keeps bars readable in a terminal-sized display.
    float normalized = std::log1p(energy) / 8.0f;
    bars[bar] = Clamp01(normalized);
  }

  for (uint32_t i = 0; i < bar_count_; ++i) {
    smoothed_bars_[i] =
        smooth_factor_ * smoothed_bars_[i] + (1.0f - smooth_factor_) * bars[i];
  }
  return smoothed_bars_;
}

// Resample mono window into fixed-count waveform points for terminal drawing.
std::vector<float> SpectrumAnalyzer::ComputeWaveform(
    const std::vector<float>& mono_window, uint32_t points) const {
  std::vector<float> waveform(points, 0.0f);
  if (mono_window.empty() || points == 0) {
    return waveform;
  }

  for (uint32_t i = 0; i < points; ++i) {
    uint32_t index = static_cast<uint32_t>((static_cast<double>(i) / points) *
                                           mono_window.size());
    if (index >= mono_window.size()) {
      index = static_cast<uint32_t>(mono_window.size() - 1);
    }
    waveform[i] = Clamp01((mono_window[index] + 1.0f) * 0.5f);
  }
  return waveform;
}

}  // namespace vocalplayer
