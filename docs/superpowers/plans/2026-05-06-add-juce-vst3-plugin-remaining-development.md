# Add JUCE VST3 Plugin Remaining Development Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the remaining `add-juce-vst3-plugin` OpenSpec work from task 8.3 through release readiness while keeping unavailable MIDI/stem capabilities correctly gated and preserving DAW safety.

**Architecture:** Continue with small plugin-owned services under `ACE-Step-Plugin\Source\...`, isolated UI components under `Source\UI`, and focused JUCE `UnitTest` coverage under `ACE-Step-Plugin\Tests`. MIDI and stems must be capability-gated because the current `acestep.cpp` backend exposes WAV output and FSQ audio tokens, not MIDI note events or confirmed separated stems.

**Tech Stack:** C++17, JUCE, CMake, MSVC 2022, VST3, GGML/acestep.cpp, PowerShell validation scripts, OpenSpec task tracking.

---

## Current Branch and Review Workflow

Use this worktree and branch:

```powershell
git -C "A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-add-juce-vst3-plugin-validation" status --short --branch
```

Expected branch:

```text
## feat/add-juce-vst3-plugin-8-3-onwards
```

Commit and push every completed task slice to:

```text
feat/add-juce-vst3-plugin-8-3-onwards
```

Do not fast-forward merge to `main` during this PR. PR #2 is the review surface.

After every plugin code change, run:

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-codex-tests" -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=OFF -DACESTEP_BUILD_TESTS=ON
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
ctest --test-dir "ACE-Step-Plugin\build-codex-tests" -C RelWithDebInfo --output-on-failure
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
```

Expected result: configure succeeds, `AceStepPluginTests` builds, CTest passes, and the stub VST3 bundle is created.

---

## File Structure Map

### New files to create

- `ACE-Step-Plugin\Source\MIDI\MidiNoteEvent.h`: Plain data type for explicit MIDI note events.
- `ACE-Step-Plugin\Source\MIDI\MidiFileWriter.h`: Small service interface for writing `.mid` files from explicit note events.
- `ACE-Step-Plugin\Source\MIDI\MidiFileWriter.cpp`: JUCE `MidiFile` implementation.
- `ACE-Step-Plugin\Tests\MidiFileWriter_test.cpp`: Tests for valid MIDI writing, empty note lists, sorting, and write failure.
- `ACE-Step-Plugin\Source\Stems\StemTypes.h`: Stem group and availability data types.
- `ACE-Step-Plugin\Source\Stems\StemCapability.h`: Capability helper that keeps stems unavailable until backend/post-processing evidence exists.
- `ACE-Step-Plugin\Source\Stems\StemCapability.cpp`: Stem capability implementation.
- `ACE-Step-Plugin\Tests\StemCapability_test.cpp`: Tests for default-unavailable and available-with-evidence states.
- `ACE-Step-Plugin\Source\Presets\PresetTypes.h`: Versioned preset value objects.
- `ACE-Step-Plugin\Source\Presets\PresetStore.h`: Preset persistence interface.
- `ACE-Step-Plugin\Source\Presets\PresetStore.cpp`: JSON save/load/rename/delete/migration logic.
- `ACE-Step-Plugin\Tests\PresetStore_test.cpp`: Tests for save, load, migration, invalid JSON, rename, and delete.
- `ACE-Step-Plugin\docs\stem-export-design.md`: Evidence and v1 gating decision for stems.
- `ACE-Step-Plugin\docs\host-compatibility-matrix.md`: Supported DAW matrix and host-owned limitations.
- `ACE-Step-Plugin\docs\release-readiness.md`: Final validation checklist and evidence log.

### Existing files to modify

- `ACE-Step-Plugin\CMakeLists.txt`: Register every new source and test file.
- `ACE-Step-Plugin\Source\Models\GeneratedAssetHistory.h`: Add optional MIDI path, optional stem group metadata, and grouped asset relationships.
- `ACE-Step-Plugin\Source\Engine\GenerationRequest.h`: Add stem request options and preset-covered fields only after their value types exist.
- `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.h`
- `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.cpp`: Add gated MIDI callbacks and later stem-aware labelling/export affordances.
- `ACE-Step-Plugin\Source\UI\GeneratedAssetHistoryView.h`
- `ACE-Step-Plugin\Source\UI\GeneratedAssetHistoryView.cpp`: Propagate MIDI/stem callbacks and grouped assets.
- `ACE-Step-Plugin\Source\PluginEditor.h`
- `ACE-Step-Plugin\Source\PluginEditor.cpp`: Add preset browser controls and final user-visible status messages.
- `openspec\changes\add-juce-vst3-plugin\tasks.md`: Mark tasks complete only after implementation/evidence exists.

---

## Phase 1: MIDI Writer and Export Infrastructure

### Task 1: Add MIDI note event value type and file writer

**OpenSpec coverage:** 8.3

**Files:**
- Create: `ACE-Step-Plugin\Source\MIDI\MidiNoteEvent.h`
- Create: `ACE-Step-Plugin\Source\MIDI\MidiFileWriter.h`
- Create: `ACE-Step-Plugin\Source\MIDI\MidiFileWriter.cpp`
- Test: `ACE-Step-Plugin\Tests\MidiFileWriter_test.cpp`
- Modify: `ACE-Step-Plugin\CMakeLists.txt`

- [x] **Step 1: Write the failing MIDI writer tests**

Create `ACE-Step-Plugin\Tests\MidiFileWriter_test.cpp`:

```cpp
#include "ReferenceAudioBufferTestUtils.h"

#include "../Source/MIDI/MidiFileWriter.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class MidiFileWriterTests final : public juce::UnitTest
{
public:
    MidiFileWriterTests() : juce::UnitTest("MidiFileWriter") {}

    void runTest() override
    {
        beginTest("writes valid midi file for explicit notes");
        {
            const auto tempFile = juce::File::createTempFile(".mid");
            tempFile.deleteFile();

            const std::vector<MidiNoteEvent> notes {
                MidiNoteEvent { 60, 0.0, 0.5, 96 },
                MidiNoteEvent { 64, 0.5, 0.5, 88 },
            };

            const auto result = MidiFileWriter::writeType0(tempFile, notes, 120.0);

            expect(result.success, result.errorMessage);
            expect(tempFile.existsAsFile());
            expectGreaterThan(static_cast<int>(tempFile.getSize()), 14);

            tempFile.deleteFile();
        }

        beginTest("rejects empty note list");
        {
            const auto tempFile = juce::File::createTempFile(".mid");
            tempFile.deleteFile();

            const auto result = MidiFileWriter::writeType0(tempFile, {}, 120.0);

            expect(!result.success);
            expect(result.errorMessage.containsIgnoreCase("no midi notes"));
            expect(!tempFile.existsAsFile());
        }

        beginTest("rejects invalid pitch and duration");
        {
            const auto tempFile = juce::File::createTempFile(".mid");
            tempFile.deleteFile();

            const std::vector<MidiNoteEvent> notes {
                MidiNoteEvent { 128, 0.0, 0.5, 96 },
            };

            const auto result = MidiFileWriter::writeType0(tempFile, notes, 120.0);

            expect(!result.success);
            expect(result.errorMessage.containsIgnoreCase("pitch"));
            expect(!tempFile.existsAsFile());
        }
    }
};

static MidiFileWriterTests sMidiFileWriterTests;

} // namespace acestep_plugin
```

- [x] **Step 2: Add the test to CMake and run it red**

Modify the `AceStepPluginTests` source list in `ACE-Step-Plugin\CMakeLists.txt`:

```cmake
Source/MIDI/MidiFileWriter.cpp
Source/MIDI/MidiFileWriter.h
Source/MIDI/MidiNoteEvent.h
Tests/MidiFileWriter_test.cpp
```

Run:

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-codex-tests" -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=OFF -DACESTEP_BUILD_TESTS=ON
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
```

Expected: build fails because `Source\MIDI\MidiFileWriter.h` does not exist.

- [x] **Step 3: Add the MIDI value type**

Create `ACE-Step-Plugin\Source\MIDI\MidiNoteEvent.h`:

```cpp
#pragma once

namespace acestep_plugin
{

/** A single explicit MIDI note event derived from reliable note/onset data. */
struct MidiNoteEvent
{
    int pitch = 60;
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    int velocity = 96;
};

} // namespace acestep_plugin
```

- [x] **Step 4: Add the writer interface**

Create `ACE-Step-Plugin\Source\MIDI\MidiFileWriter.h`:

```cpp
#pragma once

#include "MidiNoteEvent.h"

#include <juce_core/juce_core.h>

#include <vector>

namespace acestep_plugin
{

/** Result returned by MIDI file writing. */
struct MidiFileWriteResult
{
    bool success = false;
    juce::String errorMessage;
};

/** Writes Standard MIDI Files from explicit note events. */
class MidiFileWriter final
{
public:
    /** Write a single-track SMF Type 0 file from explicit note events. */
    static MidiFileWriteResult writeType0(
        const juce::File& destination,
        const std::vector<MidiNoteEvent>& notes,
        double bpm);
};

} // namespace acestep_plugin
```

- [x] **Step 5: Add the writer implementation**

Create `ACE-Step-Plugin\Source\MIDI\MidiFileWriter.cpp`:

```cpp
#include "MidiFileWriter.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>

namespace acestep_plugin
{

namespace
{
constexpr int ticksPerQuarterNote = 960;

bool isValidNote(const MidiNoteEvent& note) noexcept
{
    return note.pitch >= 0
        && note.pitch <= 127
        && note.velocity >= 1
        && note.velocity <= 127
        && note.startSeconds >= 0.0
        && note.durationSeconds > 0.0;
}

double secondsToTicks(double seconds, double bpm) noexcept
{
    const auto quartersPerSecond = bpm / 60.0;
    return seconds * quartersPerSecond * static_cast<double>(ticksPerQuarterNote);
}
} // namespace

MidiFileWriteResult MidiFileWriter::writeType0(
    const juce::File& destination,
    const std::vector<MidiNoteEvent>& notes,
    double bpm)
{
    if (notes.empty())
        return { false, "No MIDI notes are available for this asset." };

    if (bpm <= 0.0)
        return { false, "MIDI tempo must be greater than zero." };

    for (const auto& note : notes)
    {
        if (!isValidNote(note))
            return { false, "Invalid MIDI note pitch, velocity, start time, or duration." };
    }

    auto sortedNotes = notes;
    std::sort(sortedNotes.begin(), sortedNotes.end(), [](const auto& left, const auto& right) {
        return left.startSeconds < right.startSeconds;
    });

    juce::MidiMessageSequence sequence;
    for (const auto& note : sortedNotes)
    {
        const auto startTick = secondsToTicks(note.startSeconds, bpm);
        const auto endTick = secondsToTicks(note.startSeconds + note.durationSeconds, bpm);

        sequence.addEvent(juce::MidiMessage::noteOn(1, note.pitch, static_cast<juce::uint8>(note.velocity)),
                          startTick);
        sequence.addEvent(juce::MidiMessage::noteOff(1, note.pitch), endTick);
    }
    sequence.updateMatchedPairs();

    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(ticksPerQuarterNote);
    midiFile.addTrack(sequence);

    destination.deleteFile();
    juce::FileOutputStream stream(destination);
    if (!stream.openedOk())
        return { false, "Could not open MIDI destination for writing." };

    if (!midiFile.writeTo(stream))
        return { false, "Could not write MIDI file." };

    stream.flush();
    return { true, {} };
}

} // namespace acestep_plugin
```

- [x] **Step 6: Run MIDI writer tests green**

Run:

```powershell
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
ctest --test-dir "ACE-Step-Plugin\build-codex-tests" -C RelWithDebInfo --output-on-failure
```

Expected: `MidiFileWriter` tests pass with the rest of the test suite.

- [x] **Step 7: Mark OpenSpec task 8.3 complete and commit**

Modify `openspec\changes\add-juce-vst3-plugin\tasks.md`:

```markdown
- [x] 8.3 Implement standards-compliant `.mid` file writing for eligible generated assets.
```

Commit:

```powershell
git add "ACE-Step-Plugin\Source\MIDI" "ACE-Step-Plugin\Tests\MidiFileWriter_test.cpp" "ACE-Step-Plugin\CMakeLists.txt" "openspec\changes\add-juce-vst3-plugin\tasks.md"
git commit -m "feat(plugin): add MIDI file writer infrastructure" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

### Task 2: Add gated MIDI Save As and drag paths

**OpenSpec coverage:** 8.4

**Files:**
- Modify: `ACE-Step-Plugin\Source\Models\GeneratedAssetHistory.h`
- Modify: `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.h`
- Modify: `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.cpp`
- Modify: `ACE-Step-Plugin\Source\UI\GeneratedAssetHistoryView.h`
- Modify: `ACE-Step-Plugin\Source\UI\GeneratedAssetHistoryView.cpp`
- Test: `ACE-Step-Plugin\Tests\GeneratedAssetTile_test.cpp`

- [x] **Step 1: Write failing tests for MIDI callbacks and unavailable state**

Add these cases to `GeneratedAssetTileTests::runTest()` in `ACE-Step-Plugin\Tests\GeneratedAssetTile_test.cpp`:

```cpp
beginTest("MIDI export callback is not required when unavailable");
{
    GeneratedAssetTile tile(makeTestAsset(0));
    tile.setOnMidiSaveAs([](const GeneratedAsset&) {});
    expect(!tile.canExportMidi());
}

beginTest("MIDI export can carry a generated MIDI path");
{
    auto asset = makeTestAsset(0);
    asset.midiAvailability = MidiExportAvailability::available;
    asset.midiPath = "C:\\temp\\generated.mid";

    GeneratedAssetTile tile(asset);
    expect(tile.canExportMidi());
    expectEquals(tile.getAsset().midiPath, juce::String("C:\\temp\\generated.mid"));
}
```

Expected red failure: `setOnMidiSaveAs` and `midiPath` do not exist.

- [x] **Step 2: Add MIDI path metadata**

Modify `GeneratedAsset` in `ACE-Step-Plugin\Source\Models\GeneratedAssetHistory.h`:

```cpp
/** Absolute path to the generated MIDI file when MIDI export is available. */
juce::String midiPath;
```

Place it after `midiAvailability`.

- [x] **Step 3: Add MIDI callbacks to tile and history view**

Modify `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.h`:

```cpp
using MidiSaveAsCallback = std::function<void(const GeneratedAsset&)>;

void setOnMidiSaveAs(MidiSaveAsCallback cb) { onMidiSaveAs = std::move(cb); }

MidiSaveAsCallback onMidiSaveAs;
```

Modify the constructor in `GeneratedAssetTile.cpp`:

```cpp
midiExportButton.onClick = [this] {
    if (canExportMidi() && onMidiSaveAs)
        onMidiSaveAs(asset);
};
```

Modify `GeneratedAssetHistoryView.h`:

```cpp
void setOnMidiSaveAs(GeneratedAssetTile::MidiSaveAsCallback callback);
GeneratedAssetTile::MidiSaveAsCallback onMidiSaveAsCallback;
```

Modify `ContentComponent::refresh(...)` signature to accept the MIDI callback and set it on each tile:

```cpp
if (onMidiSaveAs)
    tile->setOnMidiSaveAs(onMidiSaveAs);
```

- [x] **Step 4: Run tests**

Run:

```powershell
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
ctest --test-dir "ACE-Step-Plugin\build-codex-tests" -C RelWithDebInfo --output-on-failure
```

Expected: all tests pass; MIDI button remains disabled unless `MidiExportAvailability::available`.

- [x] **Step 5: Mark OpenSpec task 8.4 complete and commit**

Modify `openspec\changes\add-juce-vst3-plugin\tasks.md`:

```markdown
- [x] 8.4 Add MIDI drag-and-drop and Save As export paths with copy semantics.
```

Commit:

```powershell
git add "ACE-Step-Plugin\Source\Models\GeneratedAssetHistory.h" "ACE-Step-Plugin\Source\UI\GeneratedAssetTile.h" "ACE-Step-Plugin\Source\UI\GeneratedAssetTile.cpp" "ACE-Step-Plugin\Source\UI\GeneratedAssetHistoryView.h" "ACE-Step-Plugin\Source\UI\GeneratedAssetHistoryView.cpp" "ACE-Step-Plugin\Tests\GeneratedAssetTile_test.cpp" "openspec\changes\add-juce-vst3-plugin\tasks.md"
git commit -m "feat(plugin): add gated MIDI export callbacks" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

### Task 3: Complete MIDI tests and request review

**OpenSpec coverage:** 8.5

**Files:**
- Modify: `ACE-Step-Plugin\Tests\MidiFileWriter_test.cpp`
- Modify: `ACE-Step-Plugin\Tests\ExternalFileDrag_test.cpp`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [x] **Step 1: Add explicit unavailable-state coverage**

Add a test proving no MIDI file is written when no note events are available:

```cpp
beginTest("does not create midi file for unavailable note data");
{
    const auto tempFile = juce::File::createTempFile(".mid");
    tempFile.deleteFile();

    const auto result = MidiFileWriter::writeType0(tempFile, {}, 120.0);

    expect(!result.success);
    expect(!tempFile.existsAsFile());
}
```

- [x] **Step 2: Run full standard validation**

Run:

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-codex-tests" -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=OFF -DACESTEP_BUILD_TESTS=ON
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
ctest --test-dir "ACE-Step-Plugin\build-codex-tests" -C RelWithDebInfo --output-on-failure
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
```

Expected: all four commands pass.

- [x] **Step 3: Mark OpenSpec task 8.5 complete and commit**

Modify `openspec\changes\add-juce-vst3-plugin\tasks.md`:

```markdown
- [x] 8.5 Add tests for MIDI availability gating, file writing, unavailable-state handling, and export path creation.
```

Commit:

```powershell
git add "ACE-Step-Plugin\Tests" "openspec\changes\add-juce-vst3-plugin\tasks.md"
git commit -m "test(plugin): cover MIDI writer and export gating" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

- [x] **Step 4: Request GitHub Copilot code review**

Request review on PR #2 after tasks 8.3-8.5 land because the PR will contain meaningful code and tests.

---

## Phase 2: Stem Capability and Asset Grouping

### Task 4: Verify stem backend capability and document decision

**OpenSpec coverage:** 9.1

**Files:**
- Create: `ACE-Step-Plugin\docs\stem-export-design.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [x] **Step 1: Inspect backend source for stem APIs**

Run:

```powershell
rg -n "stem|stems|separate|separation|vocal|instrumental" "ACE-Step-Plugin\External\acestep_cpp"
rg -n "audio_codes|generate_wav|output_path" "ACE-Step-Plugin\External\acestep_cpp\src"
```

Expected: record whether there is a supported full-mix-to-stems path. Do not infer stems from FSQ tokens without an explicit backend/API path.

- [x] **Step 2: Write the stem design doc**

Create `ACE-Step-Plugin\docs\stem-export-design.md`:

```markdown
# Stem Export Design

## Backend Capability Finding

The current vendored backend capability was inspected for explicit stem generation or
post-processing APIs. If no reliable stem API is present, v1 keeps stem generation gated
instead of exposing misleading files.

## v1 Product Behavior

Generated assets may contain a full mix and zero or more stem WAV files. Stem controls are
disabled unless the generation path reports reliable stem outputs.

## Implementation Boundary

Stem metadata lives on generated assets. Stem production runs only on the background worker,
never inside `processBlock`.
```

Replace the first paragraph with the exact evidence from the command output before committing.

- [x] **Step 3: Mark task 9.1 complete and commit**

Modify:

```markdown
- [x] 9.1 Verify the selected ACE-Step or post-processing path for producing separated stem WAV files from generated songs.
```

Commit:

```powershell
git add "ACE-Step-Plugin\docs\stem-export-design.md" "openspec\changes\add-juce-vst3-plugin\tasks.md"
git commit -m "docs(plugin): record stem backend capability finding" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

### Task 5: Add stem request options and capability data

**OpenSpec coverage:** 9.2

**Files:**
- Create: `ACE-Step-Plugin\Source\Stems\StemTypes.h`
- Create: `ACE-Step-Plugin\Source\Stems\StemCapability.h`
- Create: `ACE-Step-Plugin\Source\Stems\StemCapability.cpp`
- Test: `ACE-Step-Plugin\Tests\StemCapability_test.cpp`
- Modify: `ACE-Step-Plugin\Source\Engine\GenerationRequest.h`
- Modify: `ACE-Step-Plugin\CMakeLists.txt`

- [x] **Step 1: Write failing stem capability tests**

Create `ACE-Step-Plugin\Tests\StemCapability_test.cpp`:

```cpp
#include "../Source/Stems/StemCapability.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class StemCapabilityTests final : public juce::UnitTest
{
public:
    StemCapabilityTests() : juce::UnitTest("StemCapability") {}

    void runTest() override
    {
        beginTest("stems unavailable by default");
        {
            expect(!StemCapability::isAvailable(StemCapabilityState::unavailable));
        }

        beginTest("known backend support makes stems available");
        {
            expect(StemCapability::isAvailable(StemCapabilityState::available));
        }
    }
};

static StemCapabilityTests sStemCapabilityTests;

} // namespace acestep_plugin
```

- [x] **Step 2: Add stem types**

Create `ACE-Step-Plugin\Source\Stems\StemTypes.h`:

```cpp
#pragma once

#include <juce_core/juce_core.h>

#include <vector>

namespace acestep_plugin
{

enum class StemCapabilityState
{
    unavailable,
    available
};

enum class StemGroup
{
    fullMix,
    vocals,
    drums,
    bass,
    other
};

struct StemAsset
{
    StemGroup group = StemGroup::fullMix;
    juce::String outputPath;
    bool success = false;
    juce::String errorMessage;
};

} // namespace acestep_plugin
```

- [x] **Step 3: Add stem capability helper**

Create `ACE-Step-Plugin\Source\Stems\StemCapability.h`:

```cpp
#pragma once

#include "StemTypes.h"

namespace acestep_plugin
{

/** Centralizes whether stem export is enabled for a generation result. */
class StemCapability final
{
public:
    static bool isAvailable(StemCapabilityState state) noexcept;
};

} // namespace acestep_plugin
```

Create `ACE-Step-Plugin\Source\Stems\StemCapability.cpp`:

```cpp
#include "StemCapability.h"

namespace acestep_plugin
{

bool StemCapability::isAvailable(StemCapabilityState state) noexcept
{
    return state == StemCapabilityState::available;
}

} // namespace acestep_plugin
```

- [x] **Step 4: Extend GenerationRequest**

Modify `ACE-Step-Plugin\Source\Engine\GenerationRequest.h`:

```cpp
#include "../Stems/StemTypes.h"
#include <vector>
```

Add fields:

```cpp
/** Whether stem outputs should be requested when the backend supports them. */
bool stemsEnabled = false;

/** Stem groups requested by the user; empty means full mix only. */
std::vector<StemGroup> requestedStemGroups;
```

- [x] **Step 5: Register files and run tests**

Add source/test files to `ACE-Step-Plugin\CMakeLists.txt`, then run:

```powershell
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
ctest --test-dir "ACE-Step-Plugin\build-codex-tests" -C RelWithDebInfo --output-on-failure
```

- [x] **Step 6: Mark task 9.2 complete and commit**

Commit:

```powershell
git add "ACE-Step-Plugin\Source\Stems" "ACE-Step-Plugin\Source\Engine\GenerationRequest.h" "ACE-Step-Plugin\Tests\StemCapability_test.cpp" "ACE-Step-Plugin\CMakeLists.txt" "openspec\changes\add-juce-vst3-plugin\tasks.md"
git commit -m "feat(plugin): add stem request capability types" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

### Task 6: Add stem progress, grouped assets, and export handling

**OpenSpec coverage:** 9.3, 9.4, 9.5, 9.6

**Files:**
- Modify: `ACE-Step-Plugin\Source\Engine\GenerationRequest.h`
- Modify: `ACE-Step-Plugin\Source\Models\GeneratedAssetHistory.h`
- Modify: `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.h`
- Modify: `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.cpp`
- Modify: `ACE-Step-Plugin\Source\UI\GeneratedAssetHistoryView.h`
- Modify: `ACE-Step-Plugin\Source\UI\GeneratedAssetHistoryView.cpp`
- Test: `ACE-Step-Plugin\Tests\GeneratedAssetHistory_test.cpp`
- Test: `ACE-Step-Plugin\Tests\GeneratedAssetTile_test.cpp`

- [x] **Step 1: Extend asset metadata for grouped stems**

Add to `GeneratedAsset`:

```cpp
/** Stem files associated with this generation result. Empty when stems are unavailable. */
std::vector<StemAsset> stems;
```

Include:

```cpp
#include "../Stems/StemTypes.h"
```

- [x] **Step 2: Add history tests for grouped stems**

Add a test proving grouped stems are retained with their parent asset:

```cpp
beginTest("history preserves grouped stem assets");
{
    GeneratedAssetHistory history;
    auto asset = makeTestAsset(0);
    asset.stems.push_back(StemAsset { StemGroup::vocals, "C:\\temp\\vocals.wav", true, {} });

    history.add(asset);

    const auto assets = history.getAssets();
    expectEquals(static_cast<int>(assets.front().stems.size()), 1);
    expectEquals(assets.front().stems.front().outputPath, juce::String("C:\\temp\\vocals.wav"));
}
```

- [x] **Step 3: Add independent stem export callbacks**

Add a callback type to `GeneratedAssetTile`:

```cpp
using StemSaveAsCallback = std::function<void(const GeneratedAsset&, const StemAsset&)>;
```

Only render stem export controls when `asset.stems` contains successful stem files. Failed stems must be shown as unavailable text, not as draggable/exportable files.

Implemented with independent stem preview and Save As callbacks, stem-specific external drag file selection, and generated-asset temp cleanup that tracks successful stem output directories.

- [x] **Step 4: Add tests for partial stem failure**

Add a tile/history test with one successful stem and one failed stem:

```cpp
asset.stems.push_back(StemAsset { StemGroup::vocals, "C:\\temp\\vocals.wav", true, {} });
asset.stems.push_back(StemAsset { StemGroup::drums, {}, false, "Stem model failed" });
```

Expected: only the successful stem is exportable; the failure message remains available to the UI.

- [x] **Step 5: Run validation and commit**

Run standard validation, then mark tasks 9.3-9.6 complete only when progress, grouped metadata, export callbacks, and tests are present.

Commit:

```powershell
git add "ACE-Step-Plugin\Source" "ACE-Step-Plugin\Tests" "ACE-Step-Plugin\CMakeLists.txt" "openspec\changes\add-juce-vst3-plugin\tasks.md"
git commit -m "feat(plugin): add gated stem asset workflow" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

---

## Phase 3: Preset Browser

### Task 7: Add versioned preset schema and persistence service

**OpenSpec coverage:** 10.1, 10.2

**Files:**
- Create: `ACE-Step-Plugin\Source\Presets\PresetTypes.h`
- Create: `ACE-Step-Plugin\Source\Presets\PresetStore.h`
- Create: `ACE-Step-Plugin\Source\Presets\PresetStore.cpp`
- Test: `ACE-Step-Plugin\Tests\PresetStore_test.cpp`
- Modify: `ACE-Step-Plugin\CMakeLists.txt`

- [x] **Step 1: Write failing preset store tests**

Create tests for save/load/rename/delete and invalid JSON. Use `juce::File::createTempFile("presets")`, delete it, then create it as a directory.

Required assertions:

```cpp
expectEquals(loaded.name, juce::String("Ambient sketch"));
expectEquals(loaded.prompt, juce::String("warm ambient pads"));
expectEquals(loaded.schemaVersion, 1);
expect(!invalidResult.success);
```

- [x] **Step 2: Define preset value type**

Create `PresetTypes.h`:

```cpp
#pragma once

#include "../Engine/GenerationRequest.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

struct GenerationPreset
{
    int schemaVersion = 1;
    juce::String id;
    juce::String name;
    GenerationRequest request;
    bool midiExportRequested = false;
    bool stemExportRequested = false;
};

struct PresetOperationResult
{
    bool success = false;
    juce::String errorMessage;
};

} // namespace acestep_plugin
```

- [x] **Step 3: Implement `PresetStore`**

`PresetStore` should:

- Use a caller-provided directory in tests.
- Use one JSON file per preset.
- Write atomically by saving to `*.tmp` then replacing the destination.
- Reject invalid JSON with a clear error result.
- Migrate schema version 0 by setting `schemaVersion = 1` and preserving known fields.

- [x] **Step 4: Run tests and commit**

Run standard validation, mark tasks 10.1 and 10.2 complete, then commit:

```powershell
git add "ACE-Step-Plugin\Source\Presets" "ACE-Step-Plugin\Tests\PresetStore_test.cpp" "ACE-Step-Plugin\CMakeLists.txt" "openspec\changes\add-juce-vst3-plugin\tasks.md"
git commit -m "feat(plugin): add preset persistence service" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

### Task 8: Add preset browser UI without auto-generation

**OpenSpec coverage:** 10.3, 10.4, 10.5

**Files:**
- Modify: `ACE-Step-Plugin\Source\PluginEditor.h`
- Modify: `ACE-Step-Plugin\Source\PluginEditor.cpp`
- Test: `ACE-Step-Plugin\Tests\PresetStore_test.cpp`
- Test: add focused editor-safe tests only if existing test harness supports headless component checks.

- [x] **Step 1: Add browser controls to editor**

Add:

```cpp
juce::Label presetHeadingLabel;
juce::ComboBox presetListBox;
juce::TextButton presetSaveButton { "Save Preset" };
juce::TextButton presetLoadButton { "Load" };
juce::TextButton presetRenameButton { "Rename" };
juce::TextButton presetDeleteButton { "Delete" };
```

- [x] **Step 2: Wire load without generation**

Loading a preset must update prompt/lyrics/parameter fields only. It must not call `engine.submitAsync(...)` and must not start downloads.

- [x] **Step 3: Add tests for no auto-generation**

Use a fake store and fake generation submit callback. Assert load updates the editor model state and generation count remains zero:

```cpp
expectEquals(fakeGenerationSubmitCount, 0);
```

- [x] **Step 4: Run validation and commit**

Mark tasks 10.3-10.5 complete only after UI, no-auto-generation behavior, and tests exist.

---

## Phase 4: Host Compatibility and Release Validation

### Task 9: Add host compatibility matrix and fallback docs

**OpenSpec coverage:** 11.1, 11.2, 11.4

**Files:**
- Create: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Create host matrix**

Create `host-compatibility-matrix.md` with rows for:

```text
Reaper
FL Studio
Cubase
Studio One
Ableton Live
Bitwig
```

Columns:

```text
Scan/load
Pass-through
Editor layout
Capture controls
Generation UI state
WAV Save As
WAV drag/drop
MIDI gated unavailable state
Stem gated unavailable state
Preset browsing
Host-owned differences
```

- [ ] **Step 2: Document host-owned limitations**

Explicitly state that timeline insertion location, import prompts, plugin scanning UX, and drag interpretation are owned by the host. The plugin-owned fallback is always Save As.

- [ ] **Step 3: Commit documentation**

Mark tasks 11.1, 11.2, and 11.4 complete after the document exists. Leave 11.3 incomplete until real host validation evidence exists.

### Task 10: Perform manual host compatibility validation

**OpenSpec coverage:** 2.7, 11.3, 12.4

**Files:**
- Modify: `ACE-Step-Plugin\docs\validate-host-load.md`
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Build stub VST3**

Run:

```powershell
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
```

- [ ] **Step 2: Validate in AudioPluginHost and Reaper**

Use `ACE-Step-Plugin\docs\validate-host-load.md`. Record:

- host name/version
- bundle path
- scan result
- editor open result
- pass-through audio result
- WAV drag/drop result
- Save As fallback result

- [ ] **Step 3: Validate supported DAWs**

Repeat for FL Studio, Cubase, Studio One, Ableton Live, and Bitwig where installed.

- [ ] **Step 4: Mark only evidence-backed tasks complete**

Mark 2.7, 11.3, and 12.4 complete only for validations actually performed and recorded.

### Task 11: Real backend and smoke validation

**OpenSpec coverage:** 3.7, 6.9, 12.3

**Files:**
- Modify: `ACE-Step-Plugin\docs\release-readiness.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Build real backend**

Run only on a machine with CUDA Toolkit 12.8, Vulkan SDK, model files, and compatible GPU:

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-real" -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=ON -DACESTEP_BUILD_REAL_SMOKE_TEST=ON -DACESTEP_BUILD_TESTS=OFF
cmake --build "ACE-Step-Plugin\build-real" --config RelWithDebInfo --parallel
.\ACE-Step-Plugin\scripts\validate-bundle.ps1
```

- [ ] **Step 2: Inspect dependencies**

Run:

```powershell
dumpbin /dependents "ACE-Step-Plugin\build-real\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3\Contents\x86_64-win\ACE-Step.dll"
```

Expected: plugin DLL dependencies and GGML runtime DLL siblings are recorded.

- [ ] **Step 3: Run real smoke test**

Run:

```powershell
.\ACE-Step-Plugin\build-real\RelWithDebInfo\AceStepRealSmokeTest.exe
```

Expected: turbo generation writes a WAV and exits 0. If model files are absent, do not mark 6.9 complete.

### Task 12: Final release readiness checks and docs update

**OpenSpec coverage:** 12.1, 12.2, 12.5, 12.6

**Files:**
- Create or modify: `ACE-Step-Plugin\docs\release-readiness.md`
- Modify: `ACE-Step-Plugin\BUILD.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Verify scan safety**

Confirm plugin construction and scan do not load GGUF models. Record evidence from code paths in `PluginProcessor`, `AceStepEngine`, and model setup UI.

- [ ] **Step 2: Verify audio callback safety**

Inspect `ACE-Step-Plugin\Source\PluginProcessor.cpp::processBlock`. Confirm no allocation, logging, file I/O, mutex locking, or string construction occurs in capture logic.

- [ ] **Step 3: Verify error states surface without host crash**

Use tests or manual flows for:

```text
missing model
checksum mismatch
out of memory
backend-load failure
cancellation
generation failure
MIDI unavailable state
stem failure
preset load failure
host compatibility limitations
```

- [ ] **Step 4: Update BUILD.md**

Add final notes for:

```text
external source pins
SDK versions
model manifest details
MIDI capability and gating
stem capability and gating
preset storage location
known host limitations
```

- [ ] **Step 5: Mark release tasks complete and commit**

Only mark tasks complete when evidence is in docs or tests.

---

## Recommended Execution Order

1. Task 1: MIDI writer.
2. Task 2: Gated MIDI Save As/drag callbacks.
3. Task 3: MIDI test completion and Copilot review.
4. Task 4: Stem capability verification document.
5. Task 5: Stem request/capability data.
6. Task 6: Gated stem grouped asset workflow.
7. Task 7: Preset persistence service.
8. Task 8: Preset browser UI.
9. Task 9: Host compatibility docs.
10. Task 10: Manual host validation.
11. Task 11: Real backend validation.
12. Task 12: Release readiness docs and final evidence.

## Review Checkpoints

Request GitHub Copilot Agent Code Reviewer after:

1. MIDI tasks 8.3-8.5 are complete.
2. Stem tasks 9.1-9.6 are complete.
3. Preset tasks 10.1-10.5 are complete.
4. Final release-readiness docs and evidence are complete.

## Blocked Evidence Rules

Do not mark these tasks complete without real hardware/software evidence:

- 2.7: JUCE AudioPluginHost and Reaper host-load validation.
- 3.7: CUDA/real backend bundle and `dumpbin /dependents` evidence.
- 6.9: Real-engine smoke test with downloaded model files and GGML backends.
- 11.3: Supported DAW matrix validation.
- 12.3 and 12.4: Bundle and DAW drag/drop validation.

## Self-Review

Spec coverage:

- MIDI 8.3-8.5: covered by Tasks 1-3.
- Stems 9.1-9.6: covered by Tasks 4-6.
- Presets 10.1-10.5: covered by Tasks 7-8.
- Host compatibility 11.1-11.4: covered by Tasks 9-10.
- Release readiness 12.1-12.6: covered by Tasks 10-12.
- Blocked validation tasks 2.7, 3.7, and 6.9: covered by Tasks 10-11 with explicit evidence gates.

Placeholder scan:

- No planned task is left without a concrete file target, command, or completion gate.

Type consistency:

- `MidiExportAvailability`, `GeneratedAsset`, and `GeneratedAssetTile` names match existing code.
- New MIDI types use `MidiNoteEvent`, `MidiFileWriter`, and `MidiFileWriteResult` consistently.
- New stem types use `StemCapabilityState`, `StemGroup`, and `StemAsset` consistently.
