# VocalPlayer Architecture

## Scope

This document records the runtime architecture for the current VocalPlayer MVP
and acts as a stable reference for future features.

Implemented scope:

- File or directory input.
- Decoding + playback via miniaudio.
- Real-time spectrum (with peak-hold) and dual waveform rendering in terminal,
  analyzed independently for left/right channels (mono duplicates channel 0).
- Meter panels for RMS/Peak and low/mid/high frequency band energy per side.
- Playlist interactions (`h/l`, `j/k`, `Space`, `Enter`, mouse select/scroll).
- View-mode switching (`m`), waveform style toggle (`v`), and theme cycling (`t`).
- Optional metadata enrichment via TagLib.

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
  audioEngine --> channelWindowL[ChannelWindow_L]
  audioEngine --> channelWindowR[ChannelWindow_R]
  spectrumAnalyzer --> visualFrame[VisualFrame]
  playbackState --> visualFrame
  trackInfo --> visualFrame
  channelWindowL --> spectrumAnalyzer
  channelWindowR --> spectrumAnalyzer
  visualFrame --> visualPipeline[VisualUpdatePipeline]
  visualPipeline --> tuiRenderer
```

## Component Responsibilities

- `main.cpp`
  - Parses CLI argument and delegates control to `AppController::Run()`.
- `AppController`
  - Owns the session state machine.
  - Coordinates decode, playback, analysis, and UI intents.
- `BuildPlaylist()`
  - Resolves input path into a sorted list of supported audio files.
- `Decoder`
  - Produces `DecodedTrack` (interleaved float PCM).
  - Handles known-length and chunked fallback reads.
- `MetadataReader`
  - Creates `TrackInfo` from decoder metadata and optional TagLib tags.
- `AudioEngine`
  - Streams PCM to output device and tracks playback cursor/state.
  - Exposes pause/resume and analysis window extraction.
- `SpectrumAnalyzer`
  - Converts per-channel time windows into spectrum bars, peak-hold hints,
    waveform points, envelope waveform points, and meter values.
- `TuiRenderer`
  - Renders panelized terminal layout (header/visual area/playlist/footer).
  - Maps key/mouse input into `UiIntent` events.
  - On each draw, refreshes the playlist snapshot from `playlist_provider()` on
    the main thread; consumes the latest `VisualFrame` from `VisualUpdatePipeline`.
- `VisualUpdatePipeline`
  - Runs `frame_provider` on a worker thread, publishes under a mutex, and
    schedules coalesced redraw requests to the active `ScreenInteractive`.
  - Applies light backoff when analysis exceeds a configurable wall-time budget.
- `CoalescingRedrawGate`
  - Ensures at most one pending redraw callback is armed until the UI marks it
    flushed (used by `VisualUpdatePipeline`).

## Interface Inventory

- Application interfaces
  - `int AppController::Run(const std::string& input_path)`
  - `std::vector<std::string> BuildPlaylist(const std::string& input_path)`
- Audio interfaces
  - `DecodedTrack Decoder::DecodeFile(const std::string& path) const`
  - `TrackInfo MetadataReader::ReadTrackInfo(...) const`
  - `AudioEngine::{Load, Start, Pause, Resume, TogglePause, Stop}`
  - `PlaybackState AudioEngine::GetPlaybackState() const`
  - `std::vector<float> AudioEngine::GetRecentChannelWindow(uint32_t channel_index, uint32_t) const`
- Analysis interface
  - `std::vector<float> SpectrumAnalyzer::ComputeBars(...)`
  - `std::vector<float> SpectrumAnalyzer::ComputeWaveform(...) const`
  - `std::vector<float> SpectrumAnalyzer::ComputeWaveformEnvelope(...) const`
  - `AudioLevels SpectrumAnalyzer::ComputeLevels(...) const`
  - `std::vector<float> SpectrumAnalyzer::ComputeBandEnergies(...) const`
- UI interfaces
  - `void TuiRenderer::Run(...)`
  - `VisualUpdatePipeline` (constructed per `Run` session; worker + coalesced redraw)
  - `CoalescingRedrawGate` (at-most-one pending redraw arm until flushed)
  - `UiIntent` enum for playback/navigation intents
  - `Keybindings` + `DefaultKeybindings()` for configurable key mapping
  - `ThemeId` / `Theme` for runtime built-in theme palettes

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
    keybindings[Keybindings]
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
  keybindings --> tuiRenderer
```

## Sequence Diagram (Single Track Session)

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
  participant Pipe as VisualUpdatePipeline
  participant UI as TuiRenderer

  User->>Main: run vocalplayer inputPath
  Main->>App: Run(inputPath)
  App->>Playlist: BuildPlaylist(inputPath)
  Playlist-->>App: trackPaths
  loop eachTrack
    App->>Decoder: DecodeFile(trackPath)
    Decoder-->>App: DecodedTrack
    App->>Metadata: ReadTrackInfo(...)
    Metadata-->>App: TrackInfo
    App->>Audio: Load(decodedTrack, trackInfo)
    App->>Audio: Start()
    App->>UI: Run(frameProvider, playlistProvider, onIntent, ...)
    loop visualRefresh
      Pipe->>Audio: frame_provider reads PCM windows and playback state
      Pipe->>Analyzer: dual-channel analysis into VisualFrame
      Pipe->>Pipe: publish snapshot coalesced Post to UI
      UI->>UI: copy latest VisualFrame playlist_provider each draw
    end
    App->>Audio: Stop()
  end
  App-->>Main: exitCode
```

## Data Flow Diagram

```mermaid
flowchart LR
  fileInput[FileOrDirectoryInput] --> playlistBuilder[BuildPlaylist]
  playlistBuilder --> trackPath[TrackPath]
  trackPath --> decoder[Decoder]
  decoder --> decodedTrack[DecodedTrack_PCM]
  trackPath --> metadataReader[MetadataReader]
  metadataReader --> trackInfo[TrackInfo]
  decodedTrack --> audioEngine[AudioEngine]
  trackInfo --> audioEngine
  audioEngine --> channelWindowL[ChannelWindow_L]
  audioEngine --> channelWindowR[ChannelWindow_R]
  channelWindowL --> spectrumAnalyzer[SpectrumAnalyzer]
  channelWindowR --> spectrumAnalyzer
  spectrumAnalyzer --> channelVisuals[ChannelVisuals_L_R]
  audioEngine --> playbackState[PlaybackState]
  trackInfo --> visualFrame[VisualFrame]
  playbackState --> visualFrame
  channelVisuals --> visualFrame
  visualFrame --> visualPipeline[VisualUpdatePipeline]
  visualPipeline --> tuiRenderer[TuiRenderer]
  userInput[KeyboardAndMouseInput] --> tuiRenderer
  tuiRenderer --> uiIntent[UiIntent]
  uiIntent --> appController[AppControllerStateMachine]
  appController --> audioEngine
```

## Runtime Data Flow Notes

- Data is intentionally one-directional for rendering:
  `AudioEngine -> SpectrumAnalyzer -> VisualFrame` (assembled and published by
  `VisualUpdatePipeline` on a worker thread), then copied into `TuiRenderer` on
  each draw alongside `playlist_provider()` results.
- Control travels in the opposite direction:
  `UserInput -> TuiRenderer -> UiIntent -> AppController`.
- `VisualFrame` is immutable per tick, reducing cross-module coupling and easing
  Rust migration.
- `AudioEngine` is the single source of truth for elapsed position and play
  state (`playing`, `paused`, `finished`).

## Data Contracts

- `DecodedTrack`: interleaved float PCM plus stream format.
- `TrackInfo`: source and display metadata for active track.
- `PlaybackState`: elapsed time, duration, and runtime flags.
- `ChannelVisuals`: per-channel spectrum, waveforms, RMS/Peak, and band meters.
- `VisualFrame`: complete UI tick payload with `left`/`right` channel visuals
  plus preferred visual mode.

## Planned Evolution

- Theme system with configurable style presets.
- LRC timeline parser and lyric sync renderer.
- Beat-driven pulse effects.
- Rule-based emotion tags, then model-based inference.
