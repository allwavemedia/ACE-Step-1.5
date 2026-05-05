## ADDED Requirements

### Requirement: Generated asset history
The system SHALL keep a visible history of the most recent generated WAV assets for the plugin instance.

#### Scenario: Multiple generations complete
- **WHEN** more than one generation succeeds in a plugin instance
- **THEN** the editor displays each recent asset as an independently selectable tile up to the configured history limit

### Requirement: Waveform preview tile
The system SHALL show a waveform thumbnail, filename, duration, and playback controls for each generated WAV asset.

#### Scenario: Asset tile is displayed
- **WHEN** a generated WAV is added to history
- **THEN** its tile includes waveform and transport information derived from the file

### Requirement: Preview playback isolation
The system SHALL route preview playback through a dedicated preview path and MUST NOT alter the plugin's host audio pass-through path.

#### Scenario: User previews a generated asset
- **WHEN** the user presses Play on an asset tile
- **THEN** preview audio plays without changing captured input or the main `processBlock` pass-through behavior

### Requirement: External file drag-and-drop
The system SHALL allow generated WAV assets to be dragged from the plugin editor into DAW timelines using JUCE external file drag APIs.

#### Scenario: User drags generated WAV to DAW
- **WHEN** the user drags an asset tile into a supported DAW timeline
- **THEN** the DAW receives the generated WAV file path as a copy-style external file drag

### Requirement: Save As export fallback
The system SHALL provide a Save As action for generated WAV assets.

#### Scenario: External drag is unavailable or undesired
- **WHEN** the user chooses Save As for an asset
- **THEN** the plugin copies the generated WAV to the user-selected destination

### Requirement: Temporary asset cleanup
The system SHALL clean up plugin-owned temporary generation directories when the plugin instance is destroyed unless the user saved files elsewhere.

#### Scenario: Plugin instance is closed
- **WHEN** the host destroys the plugin instance
- **THEN** temporary WAV files owned by that instance are removed without deleting user-saved copies
