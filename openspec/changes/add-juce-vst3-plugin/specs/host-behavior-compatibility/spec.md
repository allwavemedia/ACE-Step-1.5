## ADDED Requirements

### Requirement: Supported host behavior target
The system SHALL define supported DAW hosts for v1 validation and target consistent plugin-owned behavior across those hosts.

#### Scenario: Host matrix is documented
- **WHEN** release documentation is prepared
- **THEN** it lists the DAWs used for validation and the behaviors verified in each host

### Requirement: Consistent plugin-owned behavior
The system SHALL keep pass-through audio, editor layout, capture controls, model setup, generation state, asset history, preview, Save As, and export initiation consistent across supported VST3 hosts.

#### Scenario: Plugin is tested across supported DAWs
- **WHEN** the plugin is loaded in each supported host
- **THEN** plugin-owned controls and states behave consistently within the limits of the VST3 host environment

### Requirement: Host-controlled difference handling
The system SHALL provide a fallback or documented limitation for behavior controlled by the host rather than the plugin.

#### Scenario: Host handles file drag differently
- **WHEN** a DAW does not import an external dragged file at the expected timeline location
- **THEN** the plugin still provides Save As and documentation records the known host-specific behavior

### Requirement: Host compatibility regression checks
The system SHALL include manual validation steps for Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig.

#### Scenario: Release validation runs
- **WHEN** v1 release readiness is checked
- **THEN** each named DAW is tested for scan/load, pass-through, capture controls, generation UI state, WAV export, MIDI export when available, stem export, preset browsing, and documented fallback paths
