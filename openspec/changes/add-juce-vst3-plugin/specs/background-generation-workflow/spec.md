## ADDED Requirements

### Requirement: Single-worker generation queue
The system SHALL run ACE-Step generation on a background worker pool limited to one concurrent job per plugin instance.

#### Scenario: User starts generation
- **WHEN** the user submits a generation request
- **THEN** the request runs outside the audio thread on the serialized worker

#### Scenario: User submits another request while one is running
- **WHEN** a generation job is already active
- **THEN** the new request is rejected immediately (no-op return with an error state); the user must cancel the active job before a new one can be submitted

### Requirement: Generation request parameters
The system SHALL support prompt, lyrics, duration seconds, seed, CFG scale, LM seed, scheduler, and optional reference audio in the v1 generation request.

#### Scenario: User configures generation
- **WHEN** the editor submits a request
- **THEN** the engine receives all v1 fields needed to mirror ACE-Step generation semantics

### Requirement: Worker-side preprocessing
The system SHALL resample captured reference audio to the ACE-Step expected sample rate on the worker thread, not on the audio thread.

#### Scenario: Captured reference sample rate differs
- **WHEN** host audio is captured at a sample rate different from ACE-Step input expectations
- **THEN** the worker resamples the snapshot before invoking the audio-conditioned generation path

### Requirement: Progress and cancellation
The system SHALL report generation phase and step progress to the UI and allow cancellation of active generation jobs.

#### Scenario: Generation is running
- **WHEN** the worker advances through planning, denoising, or decoding phases
- **THEN** the editor updates progress from message-thread-safe notifications

#### Scenario: User cancels generation
- **WHEN** the user requests cancellation
- **THEN** the worker stops at the next supported cancellation point and surfaces a cancelled state to the UI

### Requirement: Temporary WAV output
The system SHALL write completed generation results as 24-bit WAV files under a per-generation temporary directory.

#### Scenario: Generation succeeds
- **WHEN** ACE-Step completes synthesis
- **THEN** the plugin creates a playable WAV file and notifies the UI with its path

### Requirement: Worker error reporting
The system SHALL catch generation failures on the worker and surface user-readable errors in the editor.

#### Scenario: Generation fails from missing model or out-of-memory
- **WHEN** the backend reports a recoverable generation failure
- **THEN** the editor shows an error state without crashing the DAW host
