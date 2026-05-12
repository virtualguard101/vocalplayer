/**
 * @file spectrum_analyzer.hpp
 * @brief Declares FFT analyzer APIs for terminal visualization features.
 *
 * Key points:
 * - Configures FFT size, bar count, and smoothing policy.
 * - Exposes ComputeBars() for spectrum visualization.
 * - Exposes ComputeWaveform() for compact waveform rendering.
 */
#ifndef VOCALPLAYER_SRC_ANALYSIS_SPECTRUM_ANALYZER_HPP_
#define VOCALPLAYER_SRC_ANALYSIS_SPECTRUM_ANALYZER_HPP_

#include <cstdint>
#include <vector>

namespace vocalplayer {

/**
 * @brief Basic level metrics derived from a mono audio window.
 */
struct AudioLevels {
  /// Root-mean-square level in [0, 1].
  float rms_level = 0.0f;
  /// Peak absolute level in [0, 1].
  float peak_level = 0.0f;
};

/**
 * @brief FFT-based analyzer that converts mono windows to UI features.
 */
class SpectrumAnalyzer {
 public:
  /**
   * @brief Construct analyzer with fixed FFT and bar settings.
   *
   * @param fft_size FFT size (must be even).
   * @param bar_count Number of rendered spectrum bars.
   * @param smooth_factor Exponential smoothing factor in [0, 1).
   */
  SpectrumAnalyzer(uint32_t fft_size, uint32_t bar_count, float smooth_factor);
  /**
   * @brief Release internal FFT resources.
   */
  ~SpectrumAnalyzer();

  SpectrumAnalyzer(const SpectrumAnalyzer&) = delete;
  SpectrumAnalyzer& operator=(const SpectrumAnalyzer&) = delete;

  /**
   * @brief Compute normalized spectrum bars from a mono sample window.
   *
   * @param mono_window Input mono samples, usually from AudioEngine.
   * @return Smoothed normalized bar amplitudes.
   */
  std::vector<float> ComputeBars(const std::vector<float>& mono_window);
  /**
   * @brief Downsample mono samples into waveform points for terminal drawing.
   *
   * @param mono_window Input mono samples.
   * @param points Number of output points.
   * @return Normalized waveform points in [0, 1].
   */
  std::vector<float> ComputeWaveform(const std::vector<float>& mono_window,
                                     uint32_t points) const;
  /**
   * @brief Build a smoothed envelope waveform from absolute mono amplitudes.
   *
   * @param mono_window Input mono samples.
   * @param points Number of output points.
   * @return Normalized envelope points in [0, 1].
   */
  std::vector<float> ComputeWaveformEnvelope(
      const std::vector<float>& mono_window, uint32_t points) const;
  /**
   * @brief Compute RMS and peak levels from mono samples.
   *
   * @param mono_window Input mono samples.
   * @return AudioLevels with normalized RMS/peak metrics.
   */
  AudioLevels ComputeLevels(const std::vector<float>& mono_window) const;
  /**
   * @brief Compute coarse normalized frequency-band energies.
   *
   * @param mono_window Input mono samples.
   * @param band_count Number of output bands.
   * @return Vector of normalized energies in [0, 1].
   */
  std::vector<float> ComputeBandEnergies(const std::vector<float>& mono_window,
                                         uint32_t band_count) const;

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
