## ADDED Requirements

### Requirement: Local model detection
The system SHALL detect required ACE-Step 1.5 GGUF model files under `%LOCALAPPDATA%\AceStepPlugin\models\` during plugin setup.

#### Scenario: Models already exist locally
- **WHEN** all required GGUF files are present and pass validation
- **THEN** the editor skips the first-run downloader and marks models ready for generation

### Requirement: First-run downloader prompt
The system SHALL show a first-run setup panel with a download action when required model files are missing.

#### Scenario: Models are missing
- **WHEN** the plugin editor opens and required GGUF files are absent
- **THEN** the editor presents a model setup flow with the approximate total download size and destination directory

### Requirement: Resumable verified downloads
The system SHALL download model files from the configured Hugging Face repository using HTTP Range resume and SHA-256 verification per file.

#### Scenario: Download is interrupted
- **WHEN** a model file download stops before completion
- **THEN** the user can resume without restarting completed byte ranges

#### Scenario: Download completes
- **WHEN** a model file finishes downloading
- **THEN** the plugin verifies the file's SHA-256 before marking it usable

### Requirement: Downloader progress controls
The system SHALL expose pause, resume, per-file progress, and aggregate progress for model downloads.

#### Scenario: User pauses downloads
- **WHEN** the user activates Pause during model download
- **THEN** active transfers stop cleanly and progress remains available for resume

### Requirement: Lazy model loading
The system SHALL avoid loading GGUF model weights during plugin construction or DAW scanning.

#### Scenario: DAW scans plugin
- **WHEN** a DAW scans or instantiates the plugin without a generation request
- **THEN** the plugin does not load the 7.7 GB model set into memory
