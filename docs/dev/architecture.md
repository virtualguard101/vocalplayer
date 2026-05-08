# VocalPlayer Architecture

## Scope

This document records the runtime architecture for the current VocalPlayer MVP
and acts as a stable reference for future features.

Implemented scope:

- File or directory input.
- Decoding + playback via miniaudio.
- Real-time spectrum and waveform rendering in terminal.
- Playlist interactions (`h/l`, `j/k`, `Space`, `Enter`, mouse select/scroll).
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
  audioEngine --> monoWindow[MonoWindow]
  spectrumAnalyzer --> visualFrame[VisualFrame]
  playbackState --> visualFrame
  trackInfo --> visualFrame
  monoWindow --> spectrumAnalyzer
  visualFrame --> tuiRenderer
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
  - Converts mono windows into spectrum bars and waveform points.
- `TuiRenderer`
  - Renders the terminal view.
  - Maps key/mouse input into `UiIntent` events.

## Interface Inventory

- Application interfaces
  - `int AppController::Run(const std::string& input_path)`
  - `std::vector<std::string> BuildPlaylist(const std::string& input_path)`
- Audio interfaces
  - `DecodedTrack Decoder::DecodeFile(const std::string& path) const`
  - `TrackInfo MetadataReader::ReadTrackInfo(...) const`
  - `AudioEngine::{Load, Start, Pause, Resume, TogglePause, Stop}`
  - `PlaybackState AudioEngine::GetPlaybackState() const`
  - `std::vector<float> AudioEngine::GetRecentMonoWindow(uint32_t) const`
- Analysis interface
  - `std::vector<float> SpectrumAnalyzer::ComputeBars(...)`
  - `std::vector<float> SpectrumAnalyzer::ComputeWaveform(...) const`
- UI interfaces
  - `void TuiRenderer::Run(...)`
  - `UiIntent` enum for playback/navigation intents
  - `Keybindings` + `DefaultKeybindings()` for configurable key mapping

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
    loop refreshTick
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
  audioEngine --> monoWindow[MonoWindow]
  monoWindow --> spectrumAnalyzer[SpectrumAnalyzer]
  spectrumAnalyzer --> spectrumBars[SpectrumBars]
  spectrumAnalyzer --> waveformPoints[WaveformPoints]
  audioEngine --> playbackState[PlaybackState]
  trackInfo --> visualFrame[VisualFrame]
  playbackState --> visualFrame
  spectrumBars --> visualFrame
  waveformPoints --> visualFrame
  visualFrame --> tuiRenderer[TuiRenderer]
  userInput[KeyboardAndMouseInput] --> tuiRenderer
  tuiRenderer --> uiIntent[UiIntent]
  uiIntent --> appController[AppControllerStateMachine]
  appController --> audioEngine
```

## Runtime Data Flow Notes

- Data is intentionally one-directional for rendering:
  `AudioEngine -> SpectrumAnalyzer -> VisualFrame -> TuiRenderer`.
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
- `VisualFrame`: complete UI tick payload.

## Planned Evolution

- Theme system with configurable style presets.
- LRC timeline parser and lyric sync renderer.
- Beat-driven pulse effects.
- Rule-based emotion tags, then model-based inference.
