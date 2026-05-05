## ADDED Requirements

### Requirement: Real-time-safe reference buffer
The system SHALL allocate a stereo reference audio ring buffer at `prepareToPlay` with capacity for at least 60 seconds at 48 kHz using single-producer single-consumer semantics.

#### Scenario: Plugin prepares playback
- **WHEN** the host calls `prepareToPlay`
- **THEN** the reference buffer is allocated once and no per-block allocation is required for capture

### Requirement: Capture arm control
The system SHALL capture host-routed input audio only when an atomic arm flag is enabled by the UI.

#### Scenario: Capture is disabled
- **WHEN** playback runs while capture is not armed
- **THEN** the audio callback passes audio through without writing reference samples to the ring buffer

#### Scenario: Capture is armed
- **WHEN** playback runs while capture is armed
- **THEN** the audio callback copies input samples into the preallocated reference buffer without locks or file I/O

### Requirement: Overflow behavior
The system SHALL drop the oldest captured samples when the reference buffer exceeds capacity.

#### Scenario: Capture exceeds sixty seconds
- **WHEN** the host sends more than the configured capture capacity while capture is armed
- **THEN** the buffer retains the most recent audio and discards older samples

### Requirement: Immutable reference snapshot
The system SHALL provide a snapshot API that returns an immutable heap-owned audio buffer for background generation without blocking the audio thread.

#### Scenario: User starts generation with captured reference
- **WHEN** the user clicks Generate after arming and capturing input
- **THEN** the message thread obtains a stable snapshot that the worker can read while future audio capture continues independently

### Requirement: Host routing transparency
The system SHALL describe the capture source as host input from current routing and MUST NOT claim to detect track, bus, or master placement.

#### Scenario: Editor displays capture source
- **WHEN** the plugin editor is open
- **THEN** the capture source label communicates that the reference comes from current host routing
