## ADDED Requirements

### Requirement: In-process ACE-Step backend integration
The system SHALL integrate vendored `ServeurpersoCom/acestep.cpp` source and link the plugin against `acestep-core` for the default in-process generation mode.

#### Scenario: Default backend mode is configured
- **WHEN** CMake configures the plugin without overriding `ACESTEP_PLUGIN_MODE`
- **THEN** the plugin links to `acestep-core` and does not build `ace-server.exe` as the primary generation path

### Requirement: Contained upstream API boundary
The system SHALL isolate all direct references to `acestep.cpp` internal headers and types inside `Source/Engine/AceStepCApi.h/.cpp` or equivalent engine boundary files.

#### Scenario: Upstream API usage is reviewed
- **WHEN** maintainers inspect plugin code outside the engine boundary
- **THEN** they find calls only to the plugin-owned engine API rather than direct `acestep.cpp` pipeline types

### Requirement: Guarded public header patch
The system SHALL carry an idempotent patch that promotes required `acestep-core` include directories and adds a minimal C API shim until the equivalent upstream PR is merged.

#### Scenario: Configure applies the patch
- **WHEN** CMake configures against an upstream checkout that has not accepted the public-header change
- **THEN** the guarded patch applies successfully before plugin compilation

#### Scenario: Configure sees patch already applied
- **WHEN** CMake configures against an upstream checkout that already contains the public-header change
- **THEN** configuration continues without failing from a duplicate patch application

### Requirement: Sidecar server fallback option
The system SHALL support `-DACESTEP_PLUGIN_MODE=server` to build and bundle `ace-server.exe` as a sidecar fallback generation mode.

#### Scenario: Server mode is selected
- **WHEN** a developer configures CMake with `ACESTEP_PLUGIN_MODE=server`
- **THEN** the build depends on `ace-server` and copies the executable into the VST3 bundle resources for local loopback use

### Requirement: Bundle-local GGML backend loading
The system SHALL initialize GGML backends from the VST3 bundle directory rather than relying on the DAW process working directory.

#### Scenario: Generation initializes backends
- **WHEN** the first generation request initializes the backend
- **THEN** GGML searches the plugin bundle's `Contents/x86_64-win/` directory for CPU, CUDA, and Vulkan backend modules
