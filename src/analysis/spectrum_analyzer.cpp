/**
 * @file spectrum_analyzer.cpp
 * @brief Implements FFT-based spectrum and waveform feature extraction.
 *
 * Key points:
 * - Uses kissfft to transform mono windows into frequency-domain bins.
 * - Applies log compression and smoothing for stable bar rendering.
 * - Downsamples time-domain windows into normalized waveform points.
 */
#include "analysis/spectrum_analyzer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>

#include "kiss_fft.h"

namespace vocalplayer {
namespace {

constexpr float kPi = 3.14159265358979323846F;

// Clamp float into [0, 1] range.
float Clamp01(float value) { return std::clamp(value, 0.0F, 1.0F); }

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
      smoothed_bars_(bar_count, 0.0F),
      impl_(std::make_unique<Impl>(static_cast<int>(fft_size))) {
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

SpectrumAnalyzer::~SpectrumAnalyzer() = default;

// Compute smoothed log-compressed spectrum bars from mono window.
std::vector<float> SpectrumAnalyzer::ComputeBars(
    const std::vector<float>& mono_window) {
  std::vector<float> padded(fft_size_, 0.0F);
  if (!mono_window.empty()) {
    uint32_t copy_count = std::min<uint32_t>(mono_window.size(), fft_size_);
    std::copy_n(mono_window.end() - copy_count, copy_count,
                padded.end() - copy_count);
  }

  for (uint32_t i = 0; i < fft_size_; ++i) {
    float hann = 0.5F * (1.0F - std::cos((2.0F * kPi * i) / (fft_size_ - 1)));
    impl_->input[i].r = padded[i] * hann;
    impl_->input[i].i = 0.0F;
  }

  kiss_fft(impl_->cfg, impl_->input.data(), impl_->output.data());

  uint32_t nyquist_bins = fft_size_ / 2;
  std::vector<float> bars(bar_count_, 0.0F);
  for (uint32_t bar = 0; bar < bar_count_; ++bar) {
    uint32_t start = (bar * nyquist_bins) / bar_count_;
    uint32_t end = ((bar + 1) * nyquist_bins) / bar_count_;
    end = std::max(end, start + 1);

    float energy = 0.0F;
    for (uint32_t bin = start; bin < end; ++bin) {
      float real = impl_->output[bin].r;
      float imag = impl_->output[bin].i;
      energy += std::sqrt(real * real + imag * imag);
    }
    energy /= static_cast<float>(end - start);

    // Log compression keeps bars readable in a terminal-sized display.
    constexpr float kLogCompressionFactor = 8.0F;
    float normalized = std::log1p(energy) / kLogCompressionFactor;
    bars[bar] = Clamp01(normalized);
  }

  for (uint32_t i = 0; i < bar_count_; ++i) {
    smoothed_bars_[i] =
        smooth_factor_ * smoothed_bars_[i] + (1.0F - smooth_factor_) * bars[i];
  }
  return smoothed_bars_;
}

// Resample mono window into fixed-count waveform points for terminal drawing.
std::vector<float> SpectrumAnalyzer::ComputeWaveform(
    const std::vector<float>& mono_window, uint32_t points) const {
  std::vector<float> waveform(points, 0.0F);
  if (mono_window.empty() || points == 0) {
    return waveform;
  }

  for (uint32_t i = 0; i < points; ++i) {
    uint32_t index = static_cast<uint32_t>((static_cast<double>(i) / points) *
                                           mono_window.size());
    if (index >= mono_window.size()) {
      index = static_cast<uint32_t>(mono_window.size() - 1);
    }
    waveform[i] = Clamp01((mono_window[index] + 1.0F) * 0.5F);
  }
  return waveform;
}

// Build a compact amplitude envelope suitable for low-resolution waveform UI.
std::vector<float> SpectrumAnalyzer::ComputeWaveformEnvelope(
    const std::vector<float>& mono_window, uint32_t points) const {
  std::vector<float> envelope(points, 0.0F);
  if (mono_window.empty() || points == 0) {
    return envelope;
  }

  for (uint32_t i = 0; i < points; ++i) {
    uint32_t start = (i * static_cast<uint32_t>(mono_window.size())) / points;
    uint32_t end =
        ((i + 1) * static_cast<uint32_t>(mono_window.size())) / points;
    end = std::max(end, start + 1);
    end = std::min(end, static_cast<uint32_t>(mono_window.size()));

    float sum = 0.0F;
    for (uint32_t idx = start; idx < end; ++idx) {
      sum += std::abs(mono_window[idx]);
    }
    const float mean_abs = sum / static_cast<float>(end - start);
    envelope[i] = Clamp01(mean_abs);
  }
  return envelope;
}

// Compute RMS and peak levels from normalized mono samples.
AudioLevels SpectrumAnalyzer::ComputeLevels(
    const std::vector<float>& mono_window) const {
  AudioLevels levels;
  if (mono_window.empty()) {
    return levels;
  }

  float sum_squares = 0.0F;
  float peak_abs = 0.0F;
  for (float sample : mono_window) {
    const float abs_sample = std::abs(sample);
    peak_abs = std::max(peak_abs, abs_sample);
    sum_squares += sample * sample;
  }

  levels.rms_level =
      Clamp01(std::sqrt(sum_squares / static_cast<float>(mono_window.size())));
  levels.peak_level = Clamp01(peak_abs);
  return levels;
}

// Compute coarse per-band energies from FFT magnitudes.
std::vector<float> SpectrumAnalyzer::ComputeBandEnergies(
    const std::vector<float>& mono_window, uint32_t band_count) const {
  std::vector<float> energies(band_count, 0.0F);
  if (mono_window.empty() || band_count == 0) {
    return energies;
  }

  std::vector<float> padded(fft_size_, 0.0F);
  uint32_t copy_count = std::min<uint32_t>(mono_window.size(), fft_size_);
  std::copy_n(mono_window.end() - copy_count, copy_count,
              padded.end() - copy_count);

  for (uint32_t i = 0; i < fft_size_; ++i) {
    const float hann =
        0.5F * (1.0F - std::cos((2.0F * kPi * i) / (fft_size_ - 1)));
    impl_->input[i].r = padded[i] * hann;
    impl_->input[i].i = 0.0F;
  }
  kiss_fft(impl_->cfg, impl_->input.data(), impl_->output.data());

  const uint32_t nyquist_bins = fft_size_ / 2;
  for (uint32_t band = 0; band < band_count; ++band) {
    uint32_t start = (band * nyquist_bins) / band_count;
    uint32_t end = ((band + 1) * nyquist_bins) / band_count;
    end = std::max(end, start + 1);

    float magnitude = 0.0F;
    for (uint32_t bin = start; bin < end; ++bin) {
      const float real = impl_->output[bin].r;
      const float imag = impl_->output[bin].i;
      magnitude += std::sqrt(real * real + imag * imag);
    }

    magnitude /= static_cast<float>(end - start);
    energies[band] = Clamp01(std::log1p(magnitude) / 8.0F);
  }
  return energies;
}

}  // namespace vocalplayer
