# VocalPlayer

VocalPlayer is a creative CLI music player MVP built with C++ that focuses on
real-time rhythm visualization in the terminal.

## MVP Features

- Local audio playback (`wav` and any format supported by miniaudio decoder).
- Directory scan + simple sorted playlist playback.
- Real-time spectrum bars and waveform rendering in TUI mode.
- Track metadata display (`title`, `artist`, and duration; TagLib optional).
- Playlist interaction with Vim-style keybindings (`h/l/j/k`) and Enter confirm.

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
./build/vocalplayer /path/to/music-directory
```

Press `q` in the TUI to quit the current session.

### TUI Keybindings

- `h`: previous track
- `l`: next track
- `Space`: pause/resume current track
- `j`: move playlist selection down
- `k`: move playlist selection up
- Mouse wheel: scroll playlist viewport
- Mouse left click on playlist item: select track
- `Enter`: play selected track
- `q`: quit

### Keybinding Configuration (Reserved Interface)

`src/ui/keybindings.hpp` defines a `Keybindings` struct and
`DefaultKeybindings()`. This is the stable extension point for future
user-defined keybinding config files.

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
- Full keybinding config file loading
