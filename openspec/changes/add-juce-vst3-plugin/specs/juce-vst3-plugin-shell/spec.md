## ADDED Requirements

### Requirement: Sibling plugin repository skeleton
The system SHALL provide a sibling `ACE-Step-Plugin/` repository layout containing CMake, JUCE source, resources, external submodules, helper CMake modules, patches, VS Code configuration, and Windows build documentation.

#### Scenario: Skeleton files are generated
- **WHEN** the Step 1 implementation is complete
- **THEN** the sibling repository contains the planned root, `Source/`, `Resources/`, `External/`, `cmake/`, `patches/`, and `.vscode/` structure

### Requirement: VST3 pass-through plugin target
The system SHALL build a JUCE VST3 plugin target named `AceStepPlugin` that is stereo-in/stereo-out, is not a synth, requires no MIDI input or output, and passes input audio through unchanged.

#### Scenario: Plugin processes audio before generation features exist
- **WHEN** a DAW loads the initial plugin and sends an audio block through it
- **THEN** the plugin outputs the same audio samples without invoking ACE-Step generation

### Requirement: Windows CMake build configuration
The system SHALL configure the plugin for MSVC 2022, C++17, `/W4`, `/MP`, `/utf-8`, `/EHsc`, `JUCE_REPORT_APP_USAGE=0`, `JUCE_USE_CURL=1`, and `JUCE_DISPLAY_SPLASH_SCREEN=0`.

#### Scenario: Developer builds the VST3 target
- **WHEN** a developer runs the configured RelWithDebInfo build task on Windows with required SDKs installed
- **THEN** CMake builds the `AceStepPlugin_VST3` target using the expected compiler and JUCE definitions

### Requirement: GGML backend DLL bundling
The system SHALL copy built `ggml-*.dll` backend modules into the VST3 bundle's `Contents/x86_64-win/` directory next to the plugin binary.

#### Scenario: Bundle is inspected after build
- **WHEN** the plugin target finishes building
- **THEN** the VST3 bundle contains the plugin DLL and sibling GGML backend DLLs needed for runtime loading

### Requirement: VS Code developer workflow
The system SHALL include VS Code settings, build tasks, and debugger launch configuration for CMake Tools, clangd, and JUCE AudioPluginHost debugging.

#### Scenario: Developer opens the sibling repo in VS Code
- **WHEN** a developer opens `ACE-Step-Plugin/` in VS Code
- **THEN** they can select the MSVC kit, build `AceStepPlugin_VST3`, and attach the debugger to JUCE AudioPluginHost from the provided configuration
