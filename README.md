<div align="center">

# vocalplayer

English | [简体中文](README_zh-CN.md)

A creative C++ CLI music player with real-time rhythm visualization in the terminal.

<img src="assets/demo1.png" alt="vocalplayer demo default theme" width="32%">
<img src="assets/demo2.png" alt="vocalplayer demo miku theme" width="32%">
<img src="assets/demo3.png" alt="vocalplayer demo teto theme" width="32%">

</div>

vocalplayer is a creative CLI music player built with C++, focused on real-time rhythm visualization in terminal environments.

## Features

- Local audio playback (`wav` and formats supported by the miniaudio decoder).
- Directory scan + simple playlist sorted by name.
- Real-time spectrum bars with peak-hold markers and dual waveform modes,
  rendered independently for left/right channels (mono duplicates channel 0).
- Additional audio meters (RMS, Peak, and low/mid/high band energy) per side.
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

### Cross Build (Linux → Windows)

For quick local validation that the Windows build still configures and
compiles, a MinGW-w64 cross toolchain and a one-shot script are provided.

Prerequisites by distribution (the `wine` packages are optional and only
needed to run the produced `.exe` and ctest transparently on the Linux host):

- Arch / Manjaro / CachyOS (`pacman`):

  ```bash
  sudo pacman -S --needed mingw-w64-gcc cmake ninja
  sudo pacman -S --needed wine            # optional
  ```

- Debian / Ubuntu / Linux Mint (`apt`):

  ```bash
  sudo apt update
  sudo apt install -y mingw-w64 cmake ninja-build
  sudo apt install -y wine64              # optional (or: wine)
  ```

- Fedora / RHEL / Rocky / Alma (`dnf`):

  ```bash
  sudo dnf install -y mingw64-gcc mingw64-gcc-c++ \
                      mingw64-winpthreads-static cmake ninja-build
  sudo dnf install -y wine                # optional
  ```

- openSUSE Tumbleweed / Leap (`zypper`):

  ```bash
  sudo zypper install -y mingw64-cross-gcc-c++ cmake ninja
  sudo zypper install -y wine             # optional
  ```

- Alpine (`apk`, requires `community` repo):

  ```bash
  sudo apk add mingw-w64-gcc cmake samurai
  sudo apk add wine                       # optional
  ```

- NixOS / nix-shell (ad-hoc, no system install):

  ```bash
  nix-shell -p pkgsCross.mingwW64.buildPackages.gcc \
              cmake ninja wine
  ```

- Gentoo: use `crossdev` to build the MinGW-w64 toolchain, then install
  `dev-build/cmake`, `dev-build/ninja`, and optionally `app-emulation/wine-vanilla`:

  ```bash
  sudo emerge -av sys-devel/crossdev
  sudo crossdev --target x86_64-w64-mingw32
  sudo emerge -av dev-build/cmake dev-build/ninja
  sudo emerge -av app-emulation/wine-vanilla   # optional
  ```

The script auto-detects whichever toolchain is present as long as it exposes
the standard `x86_64-w64-mingw32-gcc` / `-g++` names; otherwise override with
`-DVOCALPLAYER_MINGW_PREFIX=<your-prefix>` when invoking CMake.

One-click cross build (uses `build-win/` so the native `build/` is untouched):

```bash
scripts/build-windows.sh                 # configure + build + ctest (if Wine present)
scripts/build-windows.sh -c -j 8         # clean rebuild with 8 parallel jobs
scripts/build-windows.sh --no-tests      # build only, skip ctest
scripts/build-windows.sh -r ~/Music      # build, then launch vocalplayer.exe via Wine
```

Or via Just:

```bash
just cross-windows                       # default Release build into build-win/
just cross-windows -- --clean            # forward flags to the script
```

Notes:

- The toolchain at `cmake/toolchains/mingw-w64-x86_64.cmake` statically links
  the MinGW C/C++ runtimes, so the resulting `build-win/vocalplayer.exe` is a
  single self-contained binary.
- TagLib is not part of the default MinGW sysroot, so the cross build runs in
  the filename-fallback metadata mode. The configure log prints
  `TagLib metadata support: DISABLED (artist/title fallback mode)` to confirm.
- TUI rendering inside `wine`/`wineconsole` is acceptable for smoke checks but
  not a substitute for a real Windows terminal; final visual verification
  should still happen on Windows.

### Contributing

Contributor-oriented workflows are documented in [`contributing.md`](contributing.md) ([简体中文](contributing_zh-CN.md)).
