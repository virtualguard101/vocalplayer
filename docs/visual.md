# VocalPlayer Visualization Metrics

This document describes the audio features currently visualized by VocalPlayer,
plus a quick-reference table for how they typically change across sections in
the same song.

## Channel layout

Metrics below are stored per side on `VisualFrame.left` and `VisualFrame.right`
(each a `ChannelVisuals`). Stereo PCM uses interleaved channels **0** and **1**
for L/R. Mono sources reuse channel **0** for both sides so layouts stay
symmetric.

## Currently Visualized Metrics

### 1) Spectrum Bars (`spectrum_bars`)

- Concept: Frequency-domain energy distribution over multiple bins.
- Intuition: Shows where energy is concentrated (low vs high frequency).
- Current behavior: FFT-based bars with smoothing and log compression.

### 2) Spectrum Peak Hold (`spectrum_peak_bars`)

- Concept: Recent short-term peak marker per spectrum bin.
- Intuition: Prevents transient peaks from disappearing immediately.
- Current behavior: Peaks decay gradually to improve readability.

### 3) Raw Waveform (`waveform_points`)

- Concept: Time-domain amplitude trace sampled into fixed-width points.
- Intuition: Reflects instantaneous signal shape and transients.
- Current behavior: Used for direct waveform rendering.

### 4) Envelope Waveform (`waveform_envelope_points`)

- Concept: Smoothed amplitude contour (mean absolute magnitude per window).
- Intuition: Highlights macro loudness contour over micro oscillation.
- Current behavior: Better for compact UI readability than raw waveform.

### 5) RMS Level (`rms_level`)

- Concept: Root-mean-square amplitude over the analysis window.
- Intuition: Represents average energy (perceived loudness trend).
- Current behavior: Normalized to [0, 1] and shown as a meter.

### 6) Peak Level (`peak_level`)

- Concept: Maximum absolute sample amplitude over the analysis window.
- Intuition: Captures short transient spikes.
- Current behavior: Normalized to [0, 1] and shown as a meter.

### 7) Band Energies (`band_energies`: Low/Mid/High)

- Concept: Coarse energy split over three frequency bands.
- Intuition: Quickly indicates tonal balance and orchestration emphasis.
- Current behavior: Normalized per band and shown as three meters.

## Quick Reference: Typical Changes Across Song Sections

This table is a practical heuristic, not a strict rule. Real tracks can vary by
genre, mixing style, and mastering.

| Song Section | Spectrum Bars | Peak Hold | Raw Waveform | Envelope | RMS | Peak | Low/Mid/High Bands |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Intro (sparse) | Fewer active bins, narrower distribution | Moderate, less dense | More gaps, isolated transients | Slow rise, lower baseline | Low to medium | Occasional spikes | Depends on arrangement; often mid/high-focused for pads or arps |
| Verse | Moderate spread, rhythm-dependent movement | Stable periodic peaks | Clear rhythmic pattern | Moderate undulation | Medium | Medium to high | Vocal-forward mixes often mid-dominant |
| Pre-chorus build | Increasing activity and width | Denser and gradually higher | Increasing density | Continuous climb | Rising | Rising, more frequent transients | Low and high often increase together during build |
| Chorus / Drop | Broad, high-energy distribution | Dense and frequent high peaks | Dense and saturated | High sustained contour | High | High | Low (kick/bass) and high (cymbals/air) often both elevated |
| Bridge / Break | Reduced density or shifted emphasis | Fewer peaks, slower decay perception | More space between events | Dip or re-shape | Medium to low | Medium | Can shift to mids for vocals/instruments, lows reduced |
| Outro | Gradual reduction in active bins | Peaks thin out | Less dense, trailing patterns | Descending contour | Falling | Falling | Returns to a narrower balance as arrangement thins |

## Practical Reading Tips

- RMS high + Peak high + dense spectrum usually means high-impact sections.
- Peak high but RMS moderate often means transient-heavy but less sustained
  energy.
- Envelope trend is useful for macro section transitions (build, drop, fade).
- Band balance often reveals arrangement focus faster than full spectrum.
