# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

vocalplayer is a creative C++ CLI music player with real-time rhythm visualization in the terminal. Built with C++20, it decodes audio via miniaudio, runs FFT analysis via kissfft, and renders a TUI with FTXUI.

## Build and Develop

```bash
# First-time setup (vcpkg install, cmake configure, clangd symlink, pre-commit hooks)
just bootstrap

# Build and run
just bd          # debug build → build/debug/
just br          # release build → build/release/

# Format
just format      # clang-format -i on src/ C++ files

# Test
just test        # full suite (requires debug build)
just qc          # quick-check: just playlist_test + keybindings_test (release build)
ctest --test-dir build/debug --output-on-failure -R "<test-name>"

# Cross-build (Linux → Windows via MinGW)
export VCPKG_ROOT=/path/to/vcpkg
just cw          # or: ./scripts/build-windows.sh
```

## Architecture

The runtime is orchestrated by `AppController::Run()` (`src/app/app_controller.cpp:41`) which wires five modules into a state machine:

```
main.cpp → AppController
             ├── Decoder          (audio file → DecodedTrack PCM)
             ├── MetadataReader   (file + stream → TrackInfo, optional TagLib)
             ├── AudioEngine      (miniaudio device playback, atomic state)
             ├── SpectrumAnalyzer (FFT → bars, waveform, levels, band energies)
             └── TuiRenderer      (FTXUI screen loop, input → UiIntent callbacks)
```

**Data flow**: `Decoder::DecodeFile()` produces `DecodedTrack` (interleaved float PCM). `AudioEngine::Load()` feeds it to a miniaudio device thread. Each render tick, `VisualUpdatePipeline` (background worker) calls into the engine and analyzer to build a `VisualFrame` snapshot, which `TuiRenderer` consumes for drawing.

**Key data contracts** are in `src/shared/types.hpp`:
- `DecodedTrack` — PCM transport between decoder and engine
- `VisualFrame` — per-render-tick snapshot (track info, playback state, per-channel visuals)
- `ChannelVisuals` — spectrum bars, waveform points, RMS/peak levels, band energies

**UI layer decoupling**: `TuiRenderer` emits `UiIntent` enum values (`kQuit`, `kPreviousTrack`, `kNextTrack`, `kTogglePause`, `kPlaySelectedTrack`) via a callback. `AppController` handles them in its control loop — the renderer knows nothing about playback internals. `UiSessionState` persists cross-track preferences (theme, visual mode, waveform style) across renderer sessions.

**Redraw coalescing**: `VisualUpdatePipeline` runs a background worker that produces `VisualFrame` snapshots and uses `CoalescingRedrawGate` (atomic compare-and-swap) to schedule at most one FTXUI `RequestAnimationFrame` at a time, avoiding event storms.

**Platform abstraction** (`src/platform/`): Two compilation units selected by `if(WIN32 OR MINGW)` in CMake — `console_posix.cpp`/`console_windows.cpp` for terminal setup and `miniaudio_decoder_open_posix.cpp`/`miniaudio_decoder_open_windows.cpp` for decoder file I/O.

## Coding Conventions

- **Google C++ Style** via `.clang-format` in repo root; CI also runs `clang-tidy` (see `.clang-tidy`).
- **Naming**: PascalCase classes/structs, camelCase functions/variables, snake_case files, ALL_CAPS constants.
- **Headers**: public API comments in `.hpp` (Doxygen `@brief`/`@param`/`@return`); `.cpp` comments only for non-obvious implementation details.
- **Modern C++**: prefer smart pointers and RAII, `std::optional` for nullable values, Rule of Five/Zero, `[[nodiscard]]` where appropriate.
- **Functions**: short (<20 lines), single-purpose, early-return over deep nesting.
- **No blank lines inside functions**.

## Pre-commit and CI

Pre-commit hooks (installed via `just bootstrap`): `clang-format` on staged C/C++ files + quick tests. GitHub Actions runs `clang-tidy` (non-blocking) and Linux build/full-tests on PRs and pushes to `main`. Releases trigger on `v*.*.*` tags pushed to remote.

## Interaction Layer Pattern

When modifying TUI interaction (keybindings, mouse, playlist):
1. Update `UiIntent` enum and state models in headers first
2. Implement control-layer state machine changes in `AppController`
3. Wire TUI event binding in `TuiRenderer` last
4. Iterate in layers: keyboard → mouse → viewport tracking → status hints

## Change Checklist

After any code change:
- `just format` the modified files
- `just bd` (or `cmake --build build/debug -j`) to verify compilation
- `ctest --test-dir build/debug --output-on-failure` to run tests
- If TUI interaction changed, smoke-test with a directory input
- Behavior changes: update both `README.md` and `README_zh-CN.md`
- Architecture changes: update both `docs/dev/architecture*.md`
- Record milestones in `changelog.md`
