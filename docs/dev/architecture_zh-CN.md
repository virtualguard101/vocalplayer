# VocalPlayer 架构文档

## 范围说明

本文档用于记录 VocalPlayer 当前 MVP 的架构设计，并作为后续迭代
（主题系统、歌词同步、情感映射）的基线文档。

当前已实现范围：

- 输入：支持单个音频文件或音频目录。
- 播放：通过 miniaudio 执行本地解码缓冲播放。
- 可视化：通过 FTXUI 实现实时频谱柱与波形渲染。
- 元数据：优先通过 TagLib 读取标题与艺术家（可选），并提供回退策略。

## 组件图

```mermaid
flowchart LR
  cliEntry[CLIEntry_main] --> appController[AppController]
  appController --> playlistBuilder[PlaylistBuilder]
  appController --> decoder[Decoder]
  appController --> metadataReader[MetadataReader]
  appController --> audioEngine[AudioEngine]
  appController --> spectrumAnalyzer[SpectrumAnalyzer]
  appController --> tuiRenderer[TuiRenderer]
  decoder --> decodedTrack[DecodedTrack]
  metadataReader --> trackInfo[TrackInfo]
  audioEngine --> playbackState[PlaybackState]
  audioEngine --> monoWindow[MonoWindow]
  spectrumAnalyzer --> visualFrame[VisualFrame]
  playbackState --> visualFrame
  trackInfo --> visualFrame
  monoWindow --> spectrumAnalyzer
  visualFrame --> tuiRenderer
```

## 时序图（单曲播放）

```mermaid
sequenceDiagram
  participant User as 用户
  participant Main as 主程序MainCLI
  participant App as 应用控制器AppController
  participant Playlist as 播放列表构建器PlaylistBuilder
  participant Decoder
  participant Metadata as 元数据读取器MetadataReader
  participant Audio as 音频引擎AudioEngine
  participant Analyzer as 频谱分析器SpectrumAnalyzer
  participant UI as 界面渲染器TuiRenderer

  User->>Main: 运行 vocalplayer inputPath
  Main->>App: Run(inputPath)
  App->>Playlist: BuildPlaylist(inputPath)
  Playlist-->>App: trackPaths
  loop 遍历每首曲目
    App->>Decoder: DecodeFile(trackPath)
    Decoder-->>App: DecodedTrack
    App->>Metadata: ReadTrackInfo(trackPath, sampleRate, channels, frames)
    Metadata-->>App: TrackInfo
    App->>Audio: Load(decodedTrack, trackInfo)
    App->>Audio: Start()
    App->>UI: Run(frameProvider, shouldStop)
    loop 每帧刷新
      UI->>Audio: GetPlaybackState()
      UI->>Audio: GetRecentMonoWindow(2048)
      UI->>Analyzer: ComputeBars(monoWindow)
      UI->>Analyzer: ComputeWaveform(monoWindow, 96)
      UI-->>UI: 渲染 VisualFrame
    end
    App->>Audio: Stop()
  end
  App-->>Main: 返回 exitCode
```

## 整体架构图

```mermaid
flowchart TB
  subgraph externalLibs [外部依赖库]
    miniaudioLib[miniaudio]
    kissfftLib[kissfft]
    ftxuiLib[FTXUI]
    taglibLib[TagLib_optional]
  end

  subgraph appLayer [应用层]
    mainCli[main.cpp]
    appController[AppController]
    playlistBuilder[PlaylistBuilder]
  end

  subgraph domainLayer [核心领域层]
    decoder[Decoder]
    metadataReader[MetadataReader]
    audioEngine[AudioEngine]
    spectrumAnalyzer[SpectrumAnalyzer]
    dataTypes[SharedTypes]
  end

  subgraph presentationLayer [表现层]
    tuiRenderer[TuiRenderer]
  end

  mainCli --> appController
  appController --> playlistBuilder
  appController --> decoder
  appController --> metadataReader
  appController --> audioEngine
  appController --> spectrumAnalyzer
  appController --> tuiRenderer

  decoder --> miniaudioLib
  audioEngine --> miniaudioLib
  spectrumAnalyzer --> kissfftLib
  tuiRenderer --> ftxuiLib
  metadataReader --> taglibLib

  decoder --> dataTypes
  audioEngine --> dataTypes
  spectrumAnalyzer --> dataTypes
  tuiRenderer --> dataTypes
```

## 数据契约

- `DecodedTrack`：交错存储的浮点采样与流格式信息。
- `TrackInfo`：来源路径、标题、艺术家、时长和采样相关元信息。
- `PlaybackState`：已播放时间、总时长及运行态标记。
- `VisualFrame`：每个刷新周期生成的 UI 只读快照。

这种“数据契约优先”的设计，使后续迁移到 Rust 时可以在保持边界稳定的前提下，
替换具体模块实现。

## 运行时说明

- 当前循环模型为“单曲渲染 + 顺序播放列表”。
- 在 TUI 中按 `q` 会退出当前会话，并停止后续曲目播放。
- 解码器同时支持已知总长度读取和分块回退读取，以兼容更多音频文件。

## 后续演进方向

- 主题系统：可配置渲染风格与配色方案。
- 歌词时间轴：LRC 解析与同步行渲染。
- 节拍脉冲：轻量级起音驱动视觉触发。
- 情感映射：先规则映射，再模型推断。
