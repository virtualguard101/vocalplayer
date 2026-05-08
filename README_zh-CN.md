# VocalPlayer

VocalPlayer 是一个使用 C++ 构建的创意型 CLI 音乐播放器 MVP，重点在于
终端中的实时节奏可视化。

## MVP 特性

- 本地音频播放（`wav` 及 miniaudio 解码器支持的格式）。
- 目录扫描 + 简单的按名称排序播放列表。
- TUI 实时频谱柱与波形渲染。
- 曲目信息展示（`title`、`artist`、时长；TagLib 可选）。

## 依赖

- CMake >= 3.20
- C++20 编译器（`clang++` 或 `g++`）
- 可选：TagLib 开发包（用于更完整的元数据读取）

CMake 会自动拉取以下第三方库：

- `miniaudio`
- `kissfft`
- `FTXUI`

## 构建

```bash
cmake -S . -B build
cmake --build build -j
```

## 运行

```bash
./build/vocalplayer /path/to/song.wav
./build/vocalplayer /path/to/music-directory
```

在 TUI 中按 `q` 可退出当前会话。

## 代码格式（Google C++ Style）

本仓库遵循 Google C++ Style。仓库根目录已提供 `.clang-format` 配置。

```bash
rg --files src -g '*.{h,hpp,cc,cpp,cxx}' -0 | xargs -0 clang-format -i
```

## 测试

```bash
ctest --test-dir build --output-on-failure
```

## 当前范围

当前 MVP 有意不包含：

- 情感分类
- 快进/切歌交互
