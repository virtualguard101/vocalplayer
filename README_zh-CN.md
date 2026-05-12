<div align="center">

# vocalplayer

[English](README.md) | 简体中文

基于C++的创意型CLI音乐播放器，在终端中实现实时节奏可视化。

<video src="assets/vocalplayer.webm" controls width="100%"></video>

</div>

vocalplayer 是一个使用 C++ 构建的创意型 CLI 音乐播放器，重点在于终端中的实时节奏可视化。

## 特性

- 本地音频播放（`wav` 及 miniaudio 解码器支持的格式）。
- 目录扫描 + 简单的按名称排序播放列表。
- TUI 实时频谱柱（含峰值保持）与双模式波形渲染。
- 增加音频信息仪表（RMS、Peak、低/中/高频段能量）。
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
- `t`：循环切换内置主题（`Default` / `Neon` / `Mono`）
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

### 构建

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build -j
```

或者：

```bash
just build
```

### 贡献

贡献流程相关内容参见[`contributing_zh-CN.md`](contributing_zh-CN.md)（[English](contributing.md)）。

