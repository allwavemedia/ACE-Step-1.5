## ADDED Requirements

### Requirement: MIDI export availability
The system SHALL expose MIDI export for generated assets when reliable note/event data is available from the backend or an implemented analysis path.

#### Scenario: MIDI data is available
- **WHEN** a generated asset contains reliable note/event data
- **THEN** the editor offers MIDI export for that asset

#### Scenario: MIDI data is unavailable
- **WHEN** a generated asset has no reliable note/event data
- **THEN** the editor shows MIDI export as unavailable for that asset without creating misleading MIDI output

### Requirement: MIDI file creation
The system SHALL create standards-compliant `.mid` files from available note/event data for user export.

#### Scenario: User exports MIDI
- **WHEN** the user chooses MIDI export for an eligible asset
- **THEN** the plugin writes a `.mid` file to a temporary or user-selected destination

### Requirement: MIDI drag-and-drop
The system SHALL allow eligible MIDI files to be dragged from the plugin editor into DAW timelines or MIDI tracks when the host accepts external MIDI file drops.

#### Scenario: User drags MIDI into a DAW
- **WHEN** the user drags an eligible MIDI asset into a supported DAW target
- **THEN** the DAW receives the `.mid` file path as a copy-style external file drag

### Requirement: MIDI Save As fallback
The system SHALL provide Save As for exported MIDI files.

#### Scenario: MIDI drag is unavailable
- **WHEN** the user chooses Save As for an eligible MIDI asset
- **THEN** the plugin copies or writes the `.mid` file to the selected destination
