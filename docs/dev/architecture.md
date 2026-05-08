# VocalPlayer Architecture

## Scope

This document records the current architecture of VocalPlayer MVP and serves as
the baseline for future iterations (theme system, lyric sync, and emotion
mapping).

Current implemented scope:

- Input: single audio file or audio directory.
- Playback: local decoded buffer playback via miniaudio.
- Visualization: real-time spectrum bars and waveform in FTXUI.
- Metadata: title and artist from TagLib (optional) with fallback strategy.

## Component Diagram

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

## Sequence Diagram (Single Track)

```mermaid
sequenceDiagram
  participant User
  participant Main as MainCLI
  participant App as AppController
  participant Playlist as PlaylistBuilder
  participant Decoder
  participant Metadata as MetadataReader
  participant Audio as AudioEngine
  participant Analyzer as SpectrumAnalyzer
  participant UI as TuiRenderer

  User->>Main: run vocalplayer inputPath
  Main->>App: Run(inputPath)
  App->>Playlist: BuildPlaylist(inputPath)
  Playlist-->>App: trackPaths
  loop eachTrack
    App->>Decoder: DecodeFile(trackPath)
    Decoder-->>App: DecodedTrack
    App->>Metadata: ReadTrackInfo(trackPath, sampleRate, channels, frames)
    Metadata-->>App: TrackInfo
    App->>Audio: Load(decodedTrack, trackInfo)
    App->>Audio: Start()
    App->>UI: Run(frameProvider, shouldStop)
    loop refreshFrame
      UI->>Audio: GetPlaybackState()
      UI->>Audio: GetRecentMonoWindow(2048)
      UI->>Analyzer: ComputeBars(monoWindow)
      UI->>Analyzer: ComputeWaveform(monoWindow, 96)
      UI-->>UI: render VisualFrame
    end
    App->>Audio: Stop()
  end
  App-->>Main: exitCode
```

## Overall Architecture Diagram

```mermaid
flowchart TB
  subgraph externalLibs [ExternalLibraries]
    miniaudioLib[miniaudio]
    kissfftLib[kissfft]
    ftxuiLib[FTXUI]
    taglibLib[TagLib_optional]
  end

  subgraph appLayer [ApplicationLayer]
    mainCli[main.cpp]
    appController[AppController]
    playlistBuilder[PlaylistBuilder]
  end

  subgraph domainLayer [CoreDomainLayer]
    decoder[Decoder]
    metadataReader[MetadataReader]
    audioEngine[AudioEngine]
    spectrumAnalyzer[SpectrumAnalyzer]
    dataTypes[SharedTypes]
  end

  subgraph presentationLayer [PresentationLayer]
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

## Data Contracts

- `DecodedTrack`: interleaved float samples and stream format info.
- `TrackInfo`: source path, title, artist, duration, sample format metadata.
- `PlaybackState`: elapsed and duration plus runtime flags.
- `VisualFrame`: UI-facing immutable snapshot assembled every refresh tick.

This contract-oriented approach allows Rust migration by replacing module
implementations while preserving stable data boundaries.

## Runtime Notes

- Current loop model is single track at a time, sequence playback for playlist.
- Pressing `q` exits current TUI session and stops remaining playlist playback.
- Decoder has both known-length and chunked fallback read paths for broader
  format compatibility.

## Planned Evolution

- Theme system: configurable rendering style and color palettes.
- Lyric timeline: LRC parsing and synchronized line rendering.
- Beat pulse: lightweight onset-driven visual triggers.
- Emotion mapping: rule-based labels first, model-based inference later.
