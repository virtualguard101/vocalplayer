# Todo List

## Iteration Plans

- v0.1.0: MVP 实现（Almost by Cursor）

  - [x] 实现最小可播放链路（解码+播放+时间进度）

  - [x] 初步实现频谱/波形可视化（简单ASCII显示）

  - [x] 实现终端渲染器与主循环刷新

  - [x] 初步实现播放列表交互（简单的上下移动和回车播放）

- [x] Code Review

- v0.2.0: 频谱/波形可视化UI增强

## Project Engineering Practices:

- [x] Formatted code with clang-format (in `v0.1.0`)

- [x] Add LSP and clangd quick setup in `justfile` (in `v0.1.0`)

- [x] Add pre-commit hooks for clang-format and tests (in `v0.1.1`)

- [x] Add clang-tidy, tests and release builds in GitHub CI/CD pipeline (in `v0.1.1`)

## Longterm Plan

- [ ] Analyzer Backend 抽象：将现有 `SpectrumAnalyzer` 拆分为前端调度层 + 后端接口（`IAnalyzerBackend`），支持 CPU/CUDA 双实现。

- [ ] CPU Baseline 固化：保留 `kissfft` 路径作为基线实现，并补充基准测试（latency/CPU usage/frame jitter）用于后续 CUDA 对比。

- [ ] CUDA 最小落地（先不接 FFT）：先将 `ComputeWaveform` 的 downsampling/normalization 迁移为独立 CUDA kernel，练习 H2D/D2H、kernel launch、错误处理与同步。

- [ ] cuFFT 接入：引入 `cuFFT` 替换频谱 FFT 主路径，完成 `window -> FFT -> magnitude/log compression -> smoothing` 的 GPU pipeline。

- [ ] 并发流水线：引入分析线程 + 双缓冲/环形缓冲（ring buffer），实现音频回调线程与分析线程解耦，避免 callback 被 GPU 同步阻塞。

- [ ] Runtime Fallback 策略：支持无 CUDA 设备、初始化失败、超时回退 CPU backend，确保可用性与可维护性。

- [ ] CMake 可选构建：增加 `ENABLE_CUDA_ANALYZER` 构建开关，默认关闭 CUDA，保持跨平台可编译。

- [ ] 观测与调优：增加 profiling 指标（kernel time、transfer time、end-to-end analyzer latency），按窗口大小（2k/4k/8k/16k）评估 GPU 收益阈值。