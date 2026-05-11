# Changelog

本文件用于记录 VocalPlayer 的迭代历史，格式参考
[Keep a Changelog](https://keepachangelog.com/)。

## [Unreleased]

## [0.1.1] - 2026-05-10

### Added
- 新增`todo.md`，用于记录待办事项与更新计划。
- 新增 `.pre-commit-config.yaml`，在提交前执行：
  - 已暂存 C/C++ 文件 `clang-format` 自动修复（修复后阻止本次提交，要求重新暂存）
  - 快速测试（`playlist_test`、`keybindings_test`）
- 新增 GitHub Actions 发布流程 `.github/workflows/release.yml`：
  - 推送 `v*.*.*` tag 时自动触发
  - 生成并上传 Linux/macOS/Windows 三平台二进制产物

### Changed
- 扩展 `justfile`：
  - `quick-check`（含 `qc` 别名）：执行本地快速门禁（配置、构建快速测试目标并运行对应测试）
  - `bootstrap`：在初始化 LSP 环境时同时安装 pre-commit hooks

### DevEx
- 新增 `.clang-tidy` 基线配置，启用 `bugprone/performance/portability/readability` 主规则集。
- 新增 `.github/workflows/ci.yml`，为 PR 与 `main` push 提供 `clang-tidy` + Linux 构建测试流水线。
- 同步更新中英文 README 的工程实践章节（pre-commit、CI、release）。

## [0.1.0] - 2026-05-08

### Added
- 初始化 C++20 + CMake 工程结构，建立 `app/audio/analysis/ui/shared` 分层。
- 接入 `miniaudio`、`kissfft`、`FTXUI`，并支持可选 `TagLib` 元数据读取。
- 实现本地音频播放链路（解码、输出、播放状态回传）。
- 实现频谱柱与波形可视化（FFT + 平滑 + TUI 渲染）。
- 支持目录扫描与简单播放列表（按文件名排序）。
- 新增播放列表交互：
  - `h/l` 上一首/下一首
  - `j/k` 选择移动
  - `Enter` 确认播放所选曲目
  - 鼠标点击选中、滚轮滚动视窗
  - `Space` 暂停/恢复
  - `q` 退出会话
- 增加“选中状态条”展示（`LIVE` / `PENDING`）与显式选中编号提示。
- 新增双语架构文档：
  - `docs/dev/architecture.md`
  - `docs/dev/architecture_zh-CN.md`

### Changed
- `AppController` 从顺序播放循环改为可中断状态机，支持运行时切歌与暂停恢复。
- `TuiRenderer` 增强为“渲染 + 输入意图桥接”模式，解耦 UI 事件与播放控制逻辑。
- 构建流程默认开启 `CMAKE_EXPORT_COMPILE_COMMANDS`，提升 clangd/LSP 体验。

### Fixed
- 修复解码阶段将 `ma_decoder_read_pcm_frames` 返回值误当帧数导致的零帧误判问题。
- 增强未知总帧数音频文件的分块回退读取逻辑。
- 修复播放列表点击与滚动视角下的索引映射问题。

### Docs
- 新增中文 README：`README_zh-CN.md`。
- README 中补充 LSP/clangd 配置与 clean 后恢复步骤，避免级联假报错。
- 为核心头文件接口补充标准化 C++ 注释（函数作用、参数、返回值与必要 Note）。

### DevEx
- 新增 `just lsp-setup` 目标，快速生成并链接 `compile_commands.json`。
- 补充 `.gitignore`，忽略 `compile_commands.json` 与 `.cursor/` 本地产物。
- 增加测试覆盖：
  - `tests/test_spectrum_analyzer.cpp`
  - `tests/test_playlist.cpp`
  - `tests/test_keybindings.cpp`
