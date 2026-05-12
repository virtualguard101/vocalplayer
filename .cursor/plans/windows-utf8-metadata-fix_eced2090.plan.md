---
name: windows-utf8-metadata-fix
overview: Implement a full Windows fix for UTF-8 display and artist metadata loading by combining runtime encoding/path handling with robust cross-platform TagLib discovery and packaging behavior.
todos:
  - id: bootstrap-windows-utf8
    content: Design Windows-only startup encoding/locale initialization in main entrypoint
    status: completed
  - id: harden-unicode-path-boundaries
    content: Plan decoder/metadata path handling changes for Unicode-safe Windows file open
    status: completed
  - id: cmake-taglib-dual-discovery
    content: Plan CMake dual TagLib discovery and compile-definition wiring
    status: completed
  - id: docs-and-changelog-sync
    content: Plan README/README_zh-CN/changelog updates for new behavior and build notes
    status: completed
  - id: validation-checklist
    content: Define build/test/manual smoke verification for CJK metadata and filenames
    status: completed
isProject: false
---

# Windows UTF-8 & Metadata Fix Plan

## Goal
Resolve two Windows regressions together:
- UTF-8 metadata (Chinese/Japanese) displays incorrectly in terminal UI.
- Artist/title metadata is often unavailable because TagLib is not reliably enabled in Windows builds.

## Scope
- Runtime: Windows console/locale initialization + Unicode-safe file path usage.
- Build: Dual TagLib discovery strategy (`find_package(TagLib CONFIG)` first, then `pkg-config` fallback).
- Validation: Build/tests + targeted manual smoke checks with CJK metadata and filenames.
- Docs/changelog sync for behavior and build expectations.

## Planned Changes

- **Runtime encoding bootstrap**
  - Update [/home/virtualguard/vg101/dev/vocalplayer/src/main.cpp](/home/virtualguard/vg101/dev/vocalplayer/src/main.cpp) to add a Windows-only initialization routine before app startup:
    - set console input/output code page to UTF-8.
    - configure C/C++ locale for UTF-8-aware output.
  - Keep non-Windows behavior unchanged.

- **Unicode-safe path flow for decoding/metadata**
  - Update [/home/virtualguard/vg101/dev/vocalplayer/src/audio/decoder.cpp](/home/virtualguard/vg101/dev/vocalplayer/src/audio/decoder.cpp) and [/home/virtualguard/vg101/dev/vocalplayer/src/audio/metadata.cpp](/home/virtualguard/vg101/dev/vocalplayer/src/audio/metadata.cpp):
    - avoid lossy Windows narrow-path assumptions when opening files.
    - use Windows-safe path conversion at boundary APIs (platform-gated).
  - Update related interfaces if needed in [/home/virtualguard/vg101/dev/vocalplayer/src/audio/decoder.hpp](/home/virtualguard/vg101/dev/vocalplayer/src/audio/decoder.hpp) and [/home/virtualguard/vg101/dev/vocalplayer/src/audio/metadata.hpp](/home/virtualguard/vg101/dev/vocalplayer/src/audio/metadata.hpp), keeping public behavior stable.

- **TagLib discovery and fallback**
  - Update [/home/virtualguard/vg101/dev/vocalplayer/CMakeLists.txt](/home/virtualguard/vg101/dev/vocalplayer/CMakeLists.txt):
    - first try `find_package(TagLib CONFIG QUIET)`.
    - if not found, fallback to existing `PkgConfig`/`taglib` flow.
    - define `VOCALPLAYER_HAS_TAGLIB` consistently for either path.
    - emit clear configure-time status messages showing whether metadata support is enabled.
  - Ensure no regression for Linux/macOS workflows.

- **Release/CI visibility (no heavy infra redesign)**
  - Review Windows pipeline hints in [/home/virtualguard/vg101/dev/vocalplayer/.github/workflows/release.yml](/home/virtualguard/vg101/dev/vocalplayer/.github/workflows/release.yml) and [/home/virtualguard/vg101/dev/vocalplayer/.github/workflows/ci.yml](/home/virtualguard/vg101/dev/vocalplayer/.github/workflows/ci.yml).
  - Add minimal diagnostics/documentation hooks rather than full dependency bootstrap in CI for this iteration.

- **Docs/changelog sync (required by repo rules)**
  - Update behavior/build notes in:
    - [/home/virtualguard/vg101/dev/vocalplayer/README.md](/home/virtualguard/vg101/dev/vocalplayer/README.md)
    - [/home/virtualguard/vg101/dev/vocalplayer/README_zh-CN.md](/home/virtualguard/vg101/dev/vocalplayer/README_zh-CN.md)
  - Add milestone entry in [/home/virtualguard/vg101/dev/vocalplayer/changelog.md](/home/virtualguard/vg101/dev/vocalplayer/changelog.md) under appropriate sections (Changed/Fixed/Docs).

## Verification Plan
- Format changed files: `clang-format -i <changed-files>`.
- Build: `cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON && cmake --build build -j`.
- Test: `ctest --test-dir build --output-on-failure`.
- Manual Windows smoke checks:
  - play a track with UTF-8 title/artist tags (Chinese/Japanese).
  - play a track whose filename/path includes CJK characters.
  - verify `artist` is loaded when TagLib is present, and fallback messaging is explicit when absent.

## Risks and Mitigations
- **Windows terminal/font variability**: document recommended terminal/font setup; keep app-side UTF-8 init explicit.
- **Library API differences across environments**: isolate platform-specific branches and preserve existing code path on non-Windows.
- **TagLib packaging variance**: dual discovery strategy plus configure-time status output for fast diagnosis.