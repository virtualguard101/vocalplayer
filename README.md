<div align="center">

# vocalplayer

English | [简体中文](README_zh-CN.md)

A creative C++ CLI music player with real-time rhythm visualization in the terminal.

[<img src="assets/demo1.png" alt="vocalplayer demo default theme" width="32%">](assets/vocalplayer.webm)
[<img src="assets/demo2.png" alt="vocalplayer demo miku theme" width="32%">](assets/vocalplayer.webm)
[<img src="assets/demo3.png" alt="vocalplayer demo teto theme" width="32%">](assets/vocalplayer.webm)

</div>

vocalplayer is a creative CLI music player built with C++, focused on real-time rhythm visualization in terminal environments.

## Features

- Local audio playback (`wav` and formats supported by the miniaudio decoder).
- Directory scan + simple playlist sorted by name.
- Real-time spectrum bars with peak-hold markers and dual waveform modes.
- Additional audio meters (RMS, Peak, and low/mid/high band energy).
- Track metadata display (`title`, `artist`, and duration; TagLib optional).
- Vim-style playlist interaction (`h/l/j/k`) and Enter-to-play confirmation.
- Panel layout mode switching (`Overview/Spectrum/Waveform/Meters`) and
  runtime built-in theme cycling.

## Usage

```bash
./vocalplayer /path/to/song.wav
./vocalplayer /path/to/music-directory
```

Press `q` in the TUI to quit the current session.

## Interaction

### Playback Interaction

- `h`: previous track
- `l`: next track
- `Space`: pause/resume current track
- `j`: move playlist selection down
- `k`: move playlist selection up
- `m`: cycle visualization layout mode
- `v`: toggle waveform style (`Raw` / `Envelope`)
- `t`: cycle built-in theme (`Default` / `Miku` / `Teto`)
- Mouse wheel: scroll playlist viewport
- Left click on a playlist item: select that track only
- `Enter`: play the currently selected track
- `q`: quit

### Keybinding Configuration Interface (Reserved)

`src/ui/keybindings.hpp` defines a `Keybindings` struct and
`DefaultKeybindings()`. Future user-defined keybinding file loading can be
built on top of this interface.

## Development

### LSP / clangd Setup

If you see many diagnostics like "header not found" in C++ files, clangd is
usually missing the compile database (`compile_commands.json`).

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -sf build/compile_commands.json compile_commands.json
```

You can also use [Just](https://github.com/casey/just):

```bash
just bootstrap
```

Then reload the IDE window so clangd can re-index.

### Dependencies

- CMake >= 3.20
- C++20 compiler (`clang++` or `g++`)
- Optional: TagLib development package (for richer metadata)

CMake automatically fetches these third-party libraries:

- `miniaudio`
- `kissfft`
- `FTXUI`

### Windows Notes

- For correct CJK metadata rendering, use a UTF-8-capable terminal and font
  (for example: Windows Terminal with a CJK font).
- CMake now tries TagLib in this order:
  1. `find_package(TagLib CONFIG)`
  2. `pkg-config taglib` fallback
- If TagLib is not found, the app still runs but falls back to filename title
  and `Unknown Artist`.

### Build

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build -j
```

or:

```bash
just build
```

### Contributing

Contributor-oriented workflows are documented in [`contributing.md`](contributing.md) ([简体中文](contributing_zh-CN.md)).
