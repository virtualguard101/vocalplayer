# VocalPlayer

VocalPlayer is a creative CLI music player MVP built with C++ that focuses on
real-time rhythm visualization in the terminal.

## MVP Features

- Local audio playback (`wav` and any format supported by miniaudio decoder).
- Real-time spectrum bars and waveform rendering in TUI mode.
- Track metadata display (`title`, `artist`, and duration; TagLib optional).

## Dependencies

- CMake >= 3.20
- C++20 compiler (`clang++` or `g++`)
- Optional: TagLib development package for richer metadata

Third-party libraries are fetched automatically by CMake:

- `miniaudio`
- `kissfft`
- `FTXUI`

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

## Run

```bash
./build/vocalplayer /path/to/song.wav
```

Press `q` in the TUI to quit.

## Formatting (Google C++ Style)

This repository follows Google C++ Style. A `.clang-format` configuration is
already present in the repository root.

```bash
rg --files src -g '*.{h,hpp,cc,cpp,cxx}' -0 | xargs -0 clang-format -i 
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

## Current Scope

The current MVP intentionally excludes:

- Emotion classification
- Playlist management
- Seek/skip interaction
