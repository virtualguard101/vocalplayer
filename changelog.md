# Changelog

本文件用于记录 vocalplayer 的迭代历史，格式参考
[Keep a Changelog](https://keepachangelog.com/)。

## [Unreleased]

### DevEx

- GitHub Actions CI：通过 `extractions/setup-just` 安装 `just`，并安装 `pre-commit`
  以满足 `justfile` 中 `bootstrap` 的 `pre-commit install`；主流程使用 `just test`、
  `clang-tidy` 任务使用 `just build-debug`，与本地 Just 入口一致。
- GitHub Actions：CI / Release 与仓库一致采用 vcpkg manifest + CMake Presets
  （`cmake --preset debug|release`，`ctest` 指向 `build/debug` / `build/release`），
  抽取 `.github/actions/vocalplayer-vcpkg-setup`；Release 中 macOS 产物标签改为
  `macos-arm64`（与 `macos-latest` 托管 Runner 架构一致）。


## [0.3.0] - 2026-05-14

### Fixed

- TUI 刷新路径：引入 `VisualUpdatePipeline` 在后台线程生成 `VisualFrame`，通过
  `CoalescingRedrawGate` 合并唤醒，使用 `Post(Closure)` + `RequestAnimationFrame()`
  替代高频 `PostEvent(Event::Custom)`，减轻与 FTXUI 内置定时任务共享队列时的积压，
  改善锁屏/解锁后可视化卡顿与时间显示迟滞。

### Changed

- TUI：频谱与波形 canvas 在各自 `window` 内水平居中（`hcenter`），避免宽面板下贴左留白。
- 立体声可视化：`AudioEngine::GetRecentChannelWindow` 按声道提取分析窗；
  `ChannelVisuals` + `VisualFrame::left`/`right` 承载左右独立频谱/波形/仪表；
  单声道曲目复制声道 0 到两侧。`GetRecentMonoWindow` 已移除。

### Docs

- 更新 `README.md`、`README_zh-CN.md`、`docs/visual.md`、`docs/visual_zh-CN.md`、
  `docs/dev/architecture.md`、`docs/dev/architecture_zh-CN.md` 以反映立体声
  可视化数据流与 `ChannelVisuals` 契约。
- 架构文档补充 `VisualUpdatePipeline` / `CoalescingRedrawGate` 与合并重绘策略说明。
- 移除 xmake 构建支持，仅保留 CMake 构建支持。

### Fixed

- 修复 Linux → Windows 交叉编译时 `pkg_check_modules(taglib)` 误识别宿主
  TagLib，导致 Linux glibc 头目录 `/usr/include` 被注入 MinGW 编译命令，
  触发 `uintptr_t`/`time_t`/`wctype_t` 等 typedef 冲突与 `fwide`/`pthread_*`
  未声明的级联错误。Toolchain 现在显式把 `PKG_CONFIG_LIBDIR`、
  `PKG_CONFIG_PATH`、`PKG_CONFIG_SYSROOT_DIR` 钉到 MinGW sysroot，
  保证 `pkg_check_modules` 只在交叉 sysroot 内搜索。

### DevEx

- 新增 `coalescing_redraw_gate_test`（`tests/test_coalescing_redraw_gate.cpp`）。
- Added `coalescing_redraw_gate_test` (`tests/test_coalescing_redraw_gate.cpp`).
- 新增 MinGW-w64 交叉编译工具链 `cmake/toolchains/mingw-w64-x86_64.cmake`，
  静态链接 MinGW 运行时，产物为单文件可分发的 `vocalplayer.exe`。
- 新增一键交叉编译脚本 `scripts/build-windows.sh`，自动检测
  `x86_64-w64-mingw32-{gcc,g++}` 与 `ninja` 是否可用，并在检测到 `wine` 时
  自动注入 `CMAKE_CROSSCOMPILING_EMULATOR`，使 `ctest` 透明运行 `.exe`。
- 扩展 `justfile`：新增 `cross-windows` 入口，并在 `clean` 中清理 `build-win/`。
- 扩展 `.gitignore`，增加 `build-*/` 通配以隔离交叉/变体构建目录。

### Docs

- 在 `README.md` 与 `README_zh-CN.md` 增加 “Cross build (Linux → Windows)”
  / “交叉编译（Linux → Windows）” 章节，说明前置依赖、一键命令与 TagLib
  在 MinGW 下回退的预期行为。

## [0.2.1] - 2026-05-12

### Changed
- 路径字符串处理统一为 UTF-8 语义（播放列表扫描、曲目显示名、元数据标题回退），降低跨平台字符集不一致导致的显示异常。
- CMake 的 TagLib 探测升级为双策略：优先 `find_package(TagLib CONFIG)`，失败后回退到 `pkg-config`。
- 配置阶段增加 TagLib 元数据能力状态提示，便于定位 `artist/title` 回退原因。

### Fixed
- Windows 启动时显式初始化 UTF-8 终端输入/输出编码与本地化环境，修复中文/日文等元数据显示异常问题。
- Windows 平台解码入口改用宽字符文件 API（`ma_decoder_init_file_w`），修复含 CJK 路径时文件打开不稳定的问题。

### Docs
- 更新 `README.md` 与 `README_zh-CN.md`，补充 Windows UTF-8 终端建议与 TagLib 查找顺序说明。

## [0.2.0] - 2026-05-12

### Added
- 新增可视化扩展数据契约：
  - `VisualFrame` 增加频谱峰值保持、包络波形、`rms_level`、`peak_level`、`band_energies` 与 `visual_mode` 字段。
- 新增分析能力：
  - `SpectrumAnalyzer::ComputeWaveformEnvelope()`
  - `SpectrumAnalyzer::ComputeLevels()`
  - `SpectrumAnalyzer::ComputeBandEnergies()`
- 新增内置主题系统：
  - `src/ui/theme.hpp` 定义 `ThemeId`、`Theme`、`GetBuiltinTheme()`、`NextThemeId()`。
  - 预留 `LoadThemeFromConfig()` 接口（暂不实现配置文件解析）。
- 新增主题测试：`tests/test_theme.cpp`（并接入 `theme_test`）。
- 新增 `xmake.lua`，提供 xmake 构建入口（含主程序、测试目标与可选 TagLib 支持）。

### Changed
- `TuiRenderer` 重构为面板化布局：顶栏、主可视化区、播放列表区、底栏。
- 频谱渲染增加峰值保持标记；波形支持 `Raw/Envelope` 双模式切换。
- 增加音频仪表展示：RMS、Peak、低中高频段能量。
- 增加运行时交互键位：
  - `m`：切换可视化布局模式
  - `v`：切换波形样式
  - `t`：切换内置主题
- 播放列表鼠标命中逻辑从固定行号偏移改为基于渲染区域 `Box` 反射定位，降低布局改动带来的点击偏移风险。
- `CMakeLists.txt` 重新格式化并补齐 `test_theme` 链接依赖（`ftxui::screen`）。

### Docs
- 同步更新 `README.md` 与 `README_zh-CN.md`，补充新增可视化、主题和交互说明。
- 同步更新 `docs/dev/architecture.md` 与 `docs/dev/architecture_zh-CN.md`，反映新的数据契约、分析接口与 UI 架构。
- 同步更新中英文 README，补充 xmake 构建与运行命令。

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
