## ADDED Requirements

### Requirement: Single-scroll generation editor
The VST3 plugin SHALL expose a single-scroll editor workflow that includes local setup status, generation controls, capture controls, progress, generated asset history, exports, presets, and diagnostics.

#### Scenario: User opens the editor
- **WHEN** the user opens the VST3 editor in a supported host
- **THEN** the editor shows setup status, prompt-to-WAV generation controls, capture controls, generated asset history, export actions, preset browsing, and diagnostics in one scrollable workflow

### Requirement: Plugin-safe sidecar orchestration
The VST3 plugin SHALL keep the audio callback pass-through-only while orchestration for ACE-Step generation, MIDI transcription, and stem separation runs in a lazy sidecar outside host scan, editor construction, and `processBlock`.

#### Scenario: Host scans the plugin
- **WHEN** a DAW scans or instantiates the plugin before explicit user generation action
- **THEN** the plugin does not launch the sidecar, load models, perform file I/O, or block the audio callback

### Requirement: Verified generated artifacts
The VST3 plugin SHALL trust sidecar outputs only after validating schema version, request ID, byte size, SHA-256, expected artifact count, and file readability.

#### Scenario: Sidecar returns a result
- **WHEN** the sidecar reports generated WAV, MIDI, or stem artifacts
- **THEN** the plugin verifies the result metadata and files before adding any success-shaped asset tile to the generated asset history

### Requirement: Functional WAV, MIDI, and stem exports
The VST3 plugin SHALL provide functional per-asset WAV export, MIDI export through Basic Pitch with `note_events.json`, and `standard-4` stem export through Demucs `htdemucs_ft`.

#### Scenario: User exports generated assets
- **WHEN** a generated full-mix asset has completed and required tools are available
- **THEN** the user can Save As or drag/drop the WAV, exported MIDI, and separated vocals, drums, bass, and other stem WAVs

### Requirement: Explicit failure surfacing
The VST3 plugin SHALL map model, sidecar, generation, MIDI, stem, preset, and host validation failures to explicit editor text without crashing or hanging the host.

#### Scenario: Export dependency fails
- **WHEN** MIDI or stem export fails after a full-mix WAV has already succeeded
- **THEN** the editor preserves the successful full-mix asset and marks the failed dependent export with explicit failure text
