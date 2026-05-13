<div align="center">

# vocalplayer

[English](README.md) | 简体中文

基于C++的创意型CLI音乐播放器，在终端中实现实时节奏可视化。

> Demo：[assets/vocalplayer.webm](assets/vocalplayer.webm)

</div>

vocalplayer 是一个使用 C++ 构建的创意型 CLI 音乐播放器，重点在于终端中的实时节奏可视化。

## 特性

- 本地音频播放（`wav` 及 miniaudio 解码器支持的格式）。
- 目录扫描 + 简单的按名称排序播放列表。
- TUI 实时频谱柱（含峰值保持）与双模式波形渲染，左右声道独立分析（单声道
  复制声道 0 到两侧）。
- 左右独立的音频信息仪表（RMS、Peak、低/中/高频段能量）。
- 曲目信息展示（`title`、`artist`、时长；TagLib 可选）。
- 支持 Vim 风格播放列表交互（`h/l/j/k`）与回车确认切歌。
- 支持可视化布局模式切换与内置主题运行时切换。

## 运行

```bash
./vocalplayer /path/to/song.wav
./vocalplayer /path/to/music-directory
```

在 TUI 中按 `q` 可退出当前会话。

## 交互

### 播放交互

- `h`：上一首
- `l`：下一首
- `Space`：暂停/恢复当前曲目
- `j`：播放列表选中下移
- `k`：播放列表选中上移
- `m`：循环切换可视化布局模式
- `v`：切换波形样式（`Raw` / `Envelope`）
- `t`：循环切换内置主题（`Default` / `Miku` / `Teto`）
- 鼠标滚轮：滚动播放列表视窗
- 鼠标左键点击列表项：仅选中该曲目
- `Enter`：播放当前选中曲目
- `q`：退出

### 键位配置接口（预留）

`src/ui/keybindings.hpp` 中定义了 `Keybindings` 结构和
`DefaultKeybindings()`。后续可在该接口基础上接入用户自定义配置文件。

## 开发

### LSP / clangd 配置

如果你在 C++ 文件里看到大量类似“header not found”的错误，通常是
clangd 没拿到编译数据库（`compile_commands.json`）。

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -sf build/compile_commands.json compile_commands.json
```

也可以使用[Just](https://github.com/casey/just)：

```bash
just bootstrap
```

随后重载 IDE 窗口，让 clangd 重新索引。

### 依赖

- CMake >= 3.20
- C++20 编译器（`clang++` 或 `g++`）
- 可选：TagLib 开发包（用于更完整的元数据读取）

CMake 会自动拉取以下第三方库：

- `miniaudio`
- `kissfft`
- `FTXUI`

### Windows 说明

- 为了正确显示中文/日文等元数据，请使用支持 UTF-8 的终端与字体
  （例如 Windows Terminal + CJK 字体）。
- CMake 现在按以下顺序查找 TagLib：
  1. `find_package(TagLib CONFIG)`
  2. `pkg-config taglib` 回退
- 若未找到 TagLib，程序仍可运行，但会回退为文件名标题和
  `Unknown Artist`。

### 构建

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build -j
```

或者：

```bash
just build
```

### 交叉编译（Linux → Windows）

为便于在 Linux 本地快速验证 Windows 构建是否仍能配置与编译，仓库提供了
MinGW-w64 交叉工具链与一键脚本。

按发行版列出的前置依赖（`wine` 系列均为可选，仅在希望本机透明运行
`.exe` 或让 ctest 直接跑 Windows 可执行文件时才需要）：

- Arch / Manjaro / CachyOS（`pacman`）：

  ```bash
  sudo pacman -S --needed mingw-w64-gcc cmake ninja
  sudo pacman -S --needed wine            # 可选
  ```

- Debian / Ubuntu / Linux Mint（`apt`）：

  ```bash
  sudo apt update
  sudo apt install -y mingw-w64 cmake ninja-build
  sudo apt install -y wine64              # 可选（也可用 wine）
  ```

- Fedora / RHEL / Rocky / Alma（`dnf`）：

  ```bash
  sudo dnf install -y mingw64-gcc mingw64-gcc-c++ \
                      mingw64-winpthreads-static cmake ninja-build
  sudo dnf install -y wine                # 可选
  ```

- openSUSE Tumbleweed / Leap（`zypper`）：

  ```bash
  sudo zypper install -y mingw64-cross-gcc-c++ cmake ninja
  sudo zypper install -y wine             # 可选
  ```

- Alpine（`apk`，需要启用 `community` 仓库）：

  ```bash
  sudo apk add mingw-w64-gcc cmake samurai
  sudo apk add wine                       # 可选
  ```

- NixOS / nix-shell（无需系统级安装）：

  ```bash
  nix-shell -p pkgsCross.mingwW64.buildPackages.gcc \
              cmake ninja wine
  ```

- Gentoo：使用 `crossdev` 构建 MinGW-w64 工具链，再安装 `cmake`、
  `ninja`，可选 `wine-vanilla`：

  ```bash
  sudo emerge -av sys-devel/crossdev
  sudo crossdev --target x86_64-w64-mingw32
  sudo emerge -av dev-build/cmake dev-build/ninja
  sudo emerge -av app-emulation/wine-vanilla   # 可选
  ```

只要工具链按标准命名暴露出 `x86_64-w64-mingw32-gcc` / `-g++`，脚本就能自动
识别；若你的发行版使用了非标准前缀，可在调用 CMake 时通过
`-DVOCALPLAYER_MINGW_PREFIX=<your-prefix>` 覆盖。

一键交叉编译（输出到 `build-win/`，不影响原生 `build/`）：

```bash
scripts/build-windows.sh                 # 配置 + 构建 + ctest（有 Wine 时）
scripts/build-windows.sh -c -j 8         # 干净重建，8 线程并行
scripts/build-windows.sh --no-tests      # 仅构建，跳过 ctest
scripts/build-windows.sh -r ~/Music      # 构建完用 Wine 直接启动 vocalplayer.exe
```

或使用 Just：

```bash
just cross-windows                       # 默认 Release 构建到 build-win/
just cross-windows -- --clean            # 把参数透传给脚本
```

说明：

- 工具链文件位于 `cmake/toolchains/mingw-w64-x86_64.cmake`，会静态链接
  MinGW 的 C/C++ 运行时，最终的 `build-win/vocalplayer.exe` 是单文件可分发产物。
- MinGW 默认 sysroot 不含 TagLib，因此交叉编译走文件名回退的元数据模式；
  配置阶段会打印 `TagLib metadata support: DISABLED (artist/title fallback mode)`
  以便确认。
- `wine` / `wineconsole` 下的 TUI 渲染可用于冒烟验证，但与真实 Windows
  终端仍有差距，UI 视觉细节最终仍需在 Windows 实机上确认。

### 贡献

贡献流程相关内容参见[`contributing_zh-CN.md`](contributing_zh-CN.md)（[English](contributing.md)）。

