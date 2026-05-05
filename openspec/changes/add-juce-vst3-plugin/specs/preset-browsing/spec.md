## ADDED Requirements

### Requirement: Preset storage
The system SHALL store plugin presets as versioned plugin-owned documents containing reusable generation settings.

#### Scenario: User saves a preset
- **WHEN** the user saves the current generation setup as a preset
- **THEN** the plugin persists prompt, lyrics, duration, seed settings, CFG scale, scheduler, reference options, stem options, MIDI options, and export preferences

### Requirement: Preset browser
The system SHALL provide a browser for listing, searching, loading, renaming, and deleting saved presets.

#### Scenario: User browses presets
- **WHEN** the preset browser opens
- **THEN** the user can find and load saved presets without relying on DAW-specific preset UI

### Requirement: Preset application
The system SHALL apply a selected preset to the editor without starting generation automatically.

#### Scenario: User loads a preset
- **WHEN** the user selects a preset
- **THEN** the editor fields update to the preset values and waits for an explicit Generate action

### Requirement: Preset schema migration
The system SHALL version preset files and migrate known older schema versions when possible.

#### Scenario: User loads an older preset
- **WHEN** the plugin opens a preset from a known older schema version
- **THEN** the plugin migrates it to current in-memory settings or reports a clear incompatibility error
