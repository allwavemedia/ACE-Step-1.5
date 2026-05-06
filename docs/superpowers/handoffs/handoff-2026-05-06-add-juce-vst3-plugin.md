# Handoff: Continue add-juce-vst3-plugin Development

## Current Objective

Continue the OpenSpec change `add-juce-vst3-plugin` for ACE-Step 1.5. The immediate development target is OpenSpec section 8, starting with task 8.3: add a standards-compliant MIDI writer infrastructure while keeping MIDI export unavailable by default because `acestep.cpp` does not expose reliable note/onset data.

## Repository and Worktree State

- Repository: `allwavemedia/ACE-Step-1.5`
- Definitive worktree for development:
  - `A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-add-juce-vst3-plugin-validation`
- Current branch:
  - `feat/add-juce-vst3-plugin-8-3-onwards`
- Current HEAD when this handoff was written:
  - `6b9c1b4b docs(plugin): plan remaining VST3 development`
- Remote branch:
  - `origin/feat/add-juce-vst3-plugin-8-3-onwards`
- Base branch:
  - `main`
- Base commit:
  - `4be44e7d feat(plugin): gate MIDI export availability`
- PR:
  - Draft PR #2: `https://github.com/allwavemedia/ACE-Step-1.5/pull/2`
- PR state:
  - Open, draft, mergeable clean
  - Contains docs-only commits so far
  - Not ready for full GitHub Copilot code review until real implementation code lands

## Recent Commits

```text
6b9c1b4b docs(plugin): plan remaining VST3 development
cda35478 docs(plugin): add MIDI export design document
4be44e7d feat(plugin): gate MIDI export availability
b1355e98 docs(plugin): record MIDI backend availability finding
ab3876bc feat(plugin): clean up generated temp directories
05068606 feat(plugin): add validation unblocking infrastructure
```

## Completed Work Relevant to This Handoff

Completed before this handoff:

- Task 7.6: Plugin-owned temp generation directory cleanup on destruction.
  - Added `GeneratedAssetTempDirectories`.
  - Wired cleanup into `PluginProcessor`.
  - Added cleanup tests.
- Task 8.1: Verified MIDI backend capability.
  - Finding: `acestep.cpp` exposes WAV generation and FSQ `audio_codes`, not MIDI note/onset events.
  - Recorded evidence in `openspec\changes\add-juce-vst3-plugin\tasks.md`.
- Task 8.2: Added per-asset MIDI availability gating.
  - Added `MidiExportAvailability`.
  - Added `GeneratedAsset::midiAvailability`, defaulting to unavailable.
  - Added disabled `MIDI N/A` button in `GeneratedAssetTile`.
  - Added tests.
- PR branch setup:
  - Created `feat/add-juce-vst3-plugin-8-3-onwards`.
  - Opened draft PR #2.
  - Added `ACE-Step-Plugin\docs\midi-export-design.md`.
  - Added implementation plan:
    - `docs\superpowers\plans\2026-05-06-add-juce-vst3-plugin-remaining-development.md`

## Important Files

- OpenSpec source of truth:
  - `openspec\changes\add-juce-vst3-plugin\tasks.md`
- Remaining development plan:
  - `docs\superpowers\plans\2026-05-06-add-juce-vst3-plugin-remaining-development.md`
- MIDI design doc:
  - `ACE-Step-Plugin\docs\midi-export-design.md`
- Current MIDI gate implementation:
  - `ACE-Step-Plugin\Source\Models\GeneratedAssetHistory.h`
  - `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.h`
  - `ACE-Step-Plugin\Source\UI\GeneratedAssetTile.cpp`
  - `ACE-Step-Plugin\Tests\GeneratedAssetTile_test.cpp`
- Build/test registration:
  - `ACE-Step-Plugin\CMakeLists.txt`

## Remaining OpenSpec Tasks

Immediate:

- 8.3: Implement standards-compliant `.mid` file writing for eligible generated assets.
- 8.4: Add MIDI drag-and-drop and Save As export paths with copy semantics.
- 8.5: Add tests for MIDI availability gating, file writing, unavailable-state handling, and export path creation.

Next:

- 9.1-9.6: Stem separation capability, gating, grouped generated assets, export handling, tests.
- 10.1-10.5: Preset JSON schema, persistence, browser UI, no-auto-generation load behavior, tests.
- 11.1-11.4: Host compatibility matrix, host-owned behavior notes, manual DAW validation, fallback docs.
- 12.1-12.6: Release readiness validation and final docs.

Evidence-gated tasks that must not be marked complete without real validation:

- 2.7: Host-load validation in JUCE AudioPluginHost and Reaper.
- 3.7: Real CUDA/backend bundle validation with `dumpbin /dependents`.
- 6.9: Real-engine smoke test with downloaded models and GGML backends.
- 11.3: Supported DAW matrix validation.
- 12.3: Bundle contains plugin DLL plus CPU/CUDA/Vulkan GGML backend DLL siblings.
- 12.4: External drag-and-drop validation across supported DAWs.

## Recommended Next Step

Start with the implementation plan's first slice:

```text
Task 1 / OpenSpec 8.3: Add MIDI note event value type and MidiFileWriter.
```

Use TDD:

1. Add failing tests in `ACE-Step-Plugin\Tests\MidiFileWriter_test.cpp`.
2. Add `ACE-Step-Plugin\Source\MIDI\MidiNoteEvent.h`.
3. Add `ACE-Step-Plugin\Source\MIDI\MidiFileWriter.h`.
4. Add `ACE-Step-Plugin\Source\MIDI\MidiFileWriter.cpp`.
5. Register sources/tests in `ACE-Step-Plugin\CMakeLists.txt`.
6. Run plugin validation.
7. Mark OpenSpec task 8.3 complete only after tests pass.
8. Commit and push to `feat/add-juce-vst3-plugin-8-3-onwards`.

Do not make MIDI export user-available by default. The writer should accept explicit note events, but generated assets should still default to `MidiExportAvailability::unavailable` until a reliable AMT or backend metadata path exists.

## Standard Validation Commands

Run after plugin code changes:

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-codex-tests" -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=OFF -DACESTEP_BUILD_TESTS=ON
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
ctest --test-dir "ACE-Step-Plugin\build-codex-tests" -C RelWithDebInfo --output-on-failure
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
```

For real-backend smoke validation only, and only when CUDA Toolkit 12.8, model files, GGML backends, and suitable hardware are available:

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-real" -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=ON -DACESTEP_BUILD_REAL_SMOKE_TEST=ON -DACESTEP_BUILD_TESTS=OFF
cmake --build "ACE-Step-Plugin\build-real" --config RelWithDebInfo --parallel
.\ACE-Step-Plugin\scripts\validate-bundle.ps1
.\ACE-Step-Plugin\build-real\RelWithDebInfo\AceStepRealSmokeTest.exe
```

## PR and Review Workflow

Use PR #2 for all remaining work:

```text
https://github.com/allwavemedia/ACE-Step-1.5/pull/2
```

Workflow:

1. Commit new work to `feat/add-juce-vst3-plugin-8-3-onwards`.
2. Push to origin.
3. Do not fast-forward merge to `main`.
4. Keep PR as draft until at least one meaningful implementation slice lands.
5. Request GitHub Copilot Agent Code Reviewer after MIDI tasks 8.3-8.5 are complete.

PR #2 is currently not ready for a full code review because it contains only documentation and planning commits. It is suitable only for a docs/plan review.

## Commit Message Convention

Use:

```text
feat(plugin): ...
fix(plugin): ...
docs(plugin): ...
test(plugin): ...
```

Always include:

```text
Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>
```

Stage explicit paths only. Do not stage `.agents`, `.github\skills`, session artifacts, temporary files, or build output.

## Key Technical Constraints

- Keep modules focused and preferably under 150 LOC; hard cap 200 LOC unless justified.
- Do not expose unfinished MIDI/stem flows as working functionality.
- Keep default MIDI and stem availability unavailable unless evidence-backed.
- Keep model loading and generation off plugin construction and off the audio thread.
- Do not add allocation, logging, file I/O, mutex locking, or string construction inside `processBlock`.
- Do not alter non-target Python/hardware runtime paths.
- Preserve Windows VST3 scope for v1.

## Suggested Opening Prompt for Next Session

```text
Continue development on add-juce-vst3-plugin using worktree:
A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-add-juce-vst3-plugin-validation

Branch:
feat/add-juce-vst3-plugin-8-3-onwards

PR:
https://github.com/allwavemedia/ACE-Step-1.5/pull/2

Read the plan:
docs\superpowers\plans\2026-05-06-add-juce-vst3-plugin-remaining-development.md

Start with Task 1 / OpenSpec 8.3: implement MidiNoteEvent and MidiFileWriter with TDD, keep MIDI unavailable by default, run the standard plugin validation commands, mark task 8.3 complete only after tests pass, commit, and push to the PR branch.
```
