#include <cassert>
#include <cmath>
#include <vector>

#include "analysis/spectrum_analyzer.hpp"

int main() {
  constexpr uint32_t kSampleCount = 2048;
  std::vector<float> signal(kSampleCount, 0.0f);

  for (uint32_t i = 0; i < kSampleCount; ++i) {
    signal[i] = std::sin(2.0 * 3.14159265358979323846 * 8.0 *
                         static_cast<double>(i) / kSampleCount);
  }

  vocalplayer::SpectrumAnalyzer analyzer(2048, 48, 0.85f);
  std::vector<float> bars = analyzer.ComputeBars(signal);
  std::vector<float> waveform = analyzer.ComputeWaveform(signal, 64);
  std::vector<float> envelope = analyzer.ComputeWaveformEnvelope(signal, 64);
  vocalplayer::AudioLevels levels = analyzer.ComputeLevels(signal);
  std::vector<float> band_energies = analyzer.ComputeBandEnergies(signal, 3);

  assert(!bars.empty());
  assert(!waveform.empty());
  assert(!envelope.empty());
  assert(band_energies.size() == 3);

  bool has_energy = false;
  for (float v : bars) {
    assert(v >= 0.0f);
    assert(v <= 1.0f);
    if (v > 0.01f) {
      has_energy = true;
    }
  }
  assert(has_energy);
  assert(levels.rms_level >= 0.0f);
  assert(levels.rms_level <= 1.0f);
  assert(levels.peak_level >= 0.0f);
  assert(levels.peak_level <= 1.0f);
  assert(levels.peak_level >= levels.rms_level);

  for (float value : envelope) {
    assert(value >= 0.0f);
    assert(value <= 1.0f);
  }
  for (float value : band_energies) {
    assert(value >= 0.0f);
    assert(value <= 1.0f);
  }

  return 0;
}
