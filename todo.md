# Todo List

## Iteration Plans

- [x] v0.1.0: MVP 实现（Almost by Cursor）

  - [x] 实现最小可播放链路（解码+播放+时间进度）

  - [x] 初步实现频谱/波形可视化（简单ASCII显示）

  - [x] 实现终端渲染器与主循环刷新

  - [x] 初步实现播放列表交互（简单的上下移动和回车播放）

- [x] Code Review

- [x] v0.2.0: 频谱/波形可视化UI增强

  - [x] 增强频谱/波形可视化UI（增加颜色、动画、交互等）

  - [x] 引入颜色主题系统（默认、预留用户自定义配置接口）

  - [x] 引入更丰富的可视化效果（如波形、频谱、波形图、频谱图等）

  - [x] 引入音频信息仪表（RMS、Peak、低/中/高频段能量）

  - [x] 引入播放器UI会话缓存机制

- [ ] v*.3.0: 引入字符艺术动态背景引擎
  
  - [ ] 引入UTF-8/ASCII字符艺术（主推泛vocaloid主题：Miku、Gumi、IA、Teto等）

  - [ ] 参考开源项目实现播放时动态字符艺术背景引擎（实现图片/视频等渲染<实时渲染还是预渲染待定>，支持从工作目录读取视频并进行解码，支持用户自定义视频源，默认携带一个teto主题和一个miku主题的ascii动态背景）

- [ ] v*.4.0: 引入ncmdump API；实现自动读取网易云本地音乐库

- [ ] v1.*.*: 

  - [ ] 将列表播放会话逻辑重构为单次UI会话，切换播放曲目时不退出UI循环，仅触发控制器换曲

## Project Engineering Practices

- [x] Formatted code with clang-format (in `v0.1.0`)

- [x] Add LSP and clangd quick setup in `justfile` (in `v0.1.0`)

- [x] Add pre-commit hooks for clang-format and tests (in `v0.1.1`)

- [x] Add clang-tidy, tests and release builds in GitHub CI/CD pipeline (in `v0.1.1`)

- [ ] Add xmake build config support

## Longterm / Debatable Plan

- [ ] 音频解码层 FFmpeg API 并行调用实现

- [ ] Rust Version: Rust并行实现

- [ ] Analyzer Backend 抽象：将现有 `SpectrumAnalyzer` 拆分为前端调度层 + 后端接口（`IAnalyzerBackend`），支持 CPU/CUDA 双实现。

- [ ] CPU Baseline 固化：保留 `kissfft` 路径作为基线实现，并补充基准测试（latency/CPU usage/frame jitter）用于后续 CUDA 对比。

- [ ] CUDA 最小落地（先不接 FFT）：先将 `ComputeWaveform` 的 downsampling/normalization 迁移为独立 CUDA kernel，练习 H2D/D2H、kernel launch、错误处理与同步。

- [ ] cuFFT 接入：引入 `cuFFT` 替换频谱 FFT 主路径，完成 `window -> FFT -> magnitude/log compression -> smoothing` 的 GPU pipeline。

- [ ] 并发流水线：引入分析线程 + 双缓冲/环形缓冲（ring buffer），实现音频回调线程与分析线程解耦，避免 callback 被 GPU 同步阻塞。

- [ ] Runtime Fallback 策略：支持无 CUDA 设备、初始化失败、超时回退 CPU backend，确保可用性与可维护性。

- [ ] CMake 可选构建：增加 `ENABLE_CUDA_ANALYZER` 构建开关，默认关闭 CUDA，保持跨平台可编译。

- [ ] 观测与调优：增加 profiling 指标（kernel time、transfer time、end-to-end analyzer latency），按窗口大小（2k/4k/8k/16k）评估 GPU 收益阈值。