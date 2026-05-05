## ADDED Requirements

### Requirement: Stem output request
The system SHALL allow users to request separated stem WAV output for generated songs.

#### Scenario: User enables stem output
- **WHEN** the user submits a generation request with stem output enabled
- **THEN** the background workflow produces a full-mix WAV plus available stem WAV files

### Requirement: Stem asset management
The system SHALL represent stem files as grouped generated assets associated with their parent full-mix generation.

#### Scenario: Stem generation succeeds
- **WHEN** a generation produces stems
- **THEN** the asset history shows the full mix and its stems as related exportable assets

### Requirement: Stem export
The system SHALL allow each stem WAV to be previewed, saved, and dragged independently.

#### Scenario: User exports a stem
- **WHEN** the user drags or saves an individual stem asset
- **THEN** the plugin exports only that selected stem WAV

### Requirement: Stem progress and errors
The system SHALL report stem generation progress and stem-specific failures separately from full-mix generation.

#### Scenario: Stem separation fails after full mix succeeds
- **WHEN** full-mix generation succeeds but stem output fails
- **THEN** the editor preserves the full-mix asset and surfaces the stem failure without crashing the host
