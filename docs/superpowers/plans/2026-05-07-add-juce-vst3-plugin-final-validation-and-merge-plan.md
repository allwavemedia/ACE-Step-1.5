# ACE-Step VST3 Final Validation and Merge Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the remaining evidence-gated `add-juce-vst3-plugin` validation tasks, update PR #2 with truthful evidence, and merge only after the remaining OpenSpec gates are satisfied.

**Architecture:** Implementation work for MIDI, stems, presets, real-backend bundling, and most release-readiness documentation is already present on PR #2. Remaining work is validation-first: run host/error-state checks, record exact evidence in plugin docs, update OpenSpec task checkboxes only when gates are met, then perform a final merge. If a host or prerequisite is unavailable, record it as blocked instead of converting it to a pass result.

**Tech Stack:** Windows VST3, JUCE 8.0.10, CMake, Visual Studio 2022, PowerShell, REAPER v7.71/x64, JUCE AudioPluginHost, CUDA Toolkit 13.2.1, Vulkan SDK 1.4.341.1, GGML/acestep.cpp, OpenSpec CLI, GitHub CLI.

---

## Current State for the Next Session

- Worktree: `A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-add-juce-vst3-plugin-validation`
- Branch: `feat/add-juce-vst3-plugin-8-3-onwards`
- PR: `https://github.com/allwavemedia/ACE-Step-1.5/pull/2`
- Latest pushed commit at plan creation: `0fdd843a docs(plugin): record AudioPluginHost partial validation`
- PR state at plan creation: open, not draft, mergeable clean, all Copilot review threads resolved.
- `origin/main` state at plan creation: already merged into the local feature branch; no merge commit was needed.
- Merge decision at plan creation: **not ready to merge** because OpenSpec evidence-gated tasks `2.7`, `11.3`, `12.4`, and `12.5` are still unchecked.

## File Structure Map

- Modify: `ACE-Step-Plugin\docs\validate-host-load.md`
  - Owns AudioPluginHost/Reaper host-load and pass-through evidence for task `2.7`.
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
  - Owns DAW matrix evidence for tasks `11.3` and `12.4`.
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`
  - Owns final checkbox state and concise evidence notes.
- Modify only if validation exposes stale instructions: `ACE-Step-Plugin\BUILD.md`
  - Owns build, SDK, and troubleshooting notes.
- Do not commit session artifacts from `C:\Users\ldoby\.copilot\session-state\...`; cite artifact paths in docs when useful.

---

## Task 1: Preflight Current Branch and PR

**Files:**
- Modify only if stale: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify only if stale: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Enter the correct worktree**

```powershell
Set-Location "A:\Repos\ACE-Step-1.5-allwavemedia.worktrees\agents-add-juce-vst3-plugin-validation"
git --no-pager branch --show-current
git --no-pager status --short --branch
git --no-pager log --oneline -5
```

Expected:

```text
feat/add-juce-vst3-plugin-8-3-onwards
## feat/add-juce-vst3-plugin-8-3-onwards
```

If `git status --short` shows uncommitted changes, inspect them with `git --no-pager diff` and preserve user edits.

- [ ] **Step 2: Confirm PR review and merge state**

```powershell
gh pr view 2 --repo allwavemedia/ACE-Step-1.5 --json `
  state,isDraft,mergeable,mergeStateStatus,reviewDecision,reviewRequests,statusCheckRollup,headRefOid,baseRefOid,url
gh api graphql -f owner=allwavemedia -f repo=ACE-Step-1.5 -F number=2 -f query='
query($owner:String!, $repo:String!, $number:Int!) {
  repository(owner:$owner, name:$repo) {
    pullRequest(number:$number) {
      reviewThreads(first:100) {
        nodes {
          isResolved
          isOutdated
          comments(first:1) { nodes { author { login } body } }
        }
      }
    }
  }
}'
```

Expected:

```text
"isDraft": false
"mergeStateStatus": "CLEAN"
"mergeable": "MERGEABLE"
```

Every non-outdated thread should have `"isResolved": true`. If a new Copilot thread is unresolved, use `receiving-code-review`, verify it against the code, fix valid feedback with tests first, reply inline, and resolve the thread.

- [ ] **Step 3: Confirm OpenSpec state**

```powershell
openspec validate add-juce-vst3-plugin --strict
Select-String -Path "openspec\changes\add-juce-vst3-plugin\tasks.md" -Pattern "^- \[ \] 2\.7|^- \[ \] 11\.3|^- \[ \] 12\.4|^- \[ \] 12\.5"
```

Expected:

```text
Change 'add-juce-vst3-plugin' is valid
```

The four evidence-gated tasks should still be unchecked until tasks 2-4 below are complete.

---

## Task 2: Complete OpenSpec 2.7 AudioPluginHost Pass-Through Evidence

**Files:**
- Modify: `ACE-Step-Plugin\docs\validate-host-load.md`
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify after both hosts pass: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Verify AudioPluginHost binary and current real bundle**

```powershell
$hostExe = "ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\AudioPluginHost.exe"
$realBundle = "C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3"
Test-Path $hostExe
Test-Path $realBundle
git rev-parse --short=12 HEAD
```

Expected:

```text
True
True
```

If either path is missing, rebuild the missing artifact before continuing:

```powershell
Set-Location "ACE-Step-Plugin\External\JUCE"
cmake -B build-audio-plugin-host -G "Visual Studio 17 2022" -A x64 -DJUCE_BUILD_EXTRAS=ON -DJUCE_BUILD_EXAMPLES=OFF
cmake --build build-audio-plugin-host --config RelWithDebInfo --target AudioPluginHost --parallel
Set-Location "..\..\.."
```

- [ ] **Step 2: Run AudioPluginHost pass-through manually**

Use `ACE-Step-Plugin\docs\validate-host-load.md`, Host A checklist:

1. Launch `AudioPluginHost.exe`.
2. Scan the VST3 directory containing `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3`.
3. Create graph: `Audio Input -> ACE-Step (VST3) -> Audio Output`.
4. Open the ACE-Step editor.
5. Play or route a non-silent audio signal for at least 5 seconds.
6. Confirm no crash, hang, silence, unexpected gain change, or audible glitching.
7. Unload/reload the plugin once and confirm the host remains stable.

- [ ] **Step 3: Record AudioPluginHost pass-through result**

Append this record to `ACE-Step-Plugin\docs\validate-host-load.md` under the AudioPluginHost status section, replacing only values observed during the run:

```markdown
### Current Real-Bundle AudioPluginHost Pass-Through Evidence

| Field | Value |
|---|---|
| Host executable | `ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\AudioPluginHost.exe` |
| OS | Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Build commit | output of `git rev-parse --short=12 HEAD` |
| Bundle path | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | `allwavemedia` |
| Result | PASS |

Manual results:
- Scan/load: PASS - ACE-Step appeared as a VST3 plugin under Allwave Media.
- Editor opens: PASS - the ACE-Step editor opened without an error dialog.
- Pass-through: PASS - non-silent input passed through for at least 5 seconds without silence, unexpected gain change, or audible glitching.
- Stability: PASS - AudioPluginHost remained stable after plugin unload/reload.
```

If the manual run fails, set `Result` to `FAIL`, record the exact failure text, do not mark task `2.7` complete, and create a fix task before proceeding to merge.

- [ ] **Step 4: Mark task 2.7 complete only after AudioPluginHost and Reaper both pass**

Edit `openspec\changes\add-juce-vst3-plugin\tasks.md`:

```markdown
- [x] 2.7 Verify the initial VST3 bundle loads in JUCE AudioPluginHost and Reaper with unchanged audio pass-through.
```

Keep the existing Reaper evidence note and add the AudioPluginHost pass-through summary from Step 3.

- [ ] **Step 5: Validate and commit task 2.7 evidence**

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\docs\validate-host-load.md ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): complete AudioPluginHost host-load validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

Expected:

```text
Change 'add-juce-vst3-plugin' is valid
```

---

## Task 3: Complete OpenSpec 11.3 and 12.4 DAW Matrix and Drag/Drop Evidence

**Files:**
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Check installed target hosts**

```powershell
$hosts = [ordered]@{
  "Reaper x64" = "C:\Program Files\REAPER (x64)\reaper.exe"
  "FL Studio 21" = "C:\Program Files\Image-Line\FL Studio 21\FL64.exe"
  "Cubase 13" = "C:\Program Files\Steinberg\Cubase 13\Cubase13.exe"
  "Studio One 6" = "C:\Program Files\PreSonus\Studio One 6\Studio One.exe"
  "Ableton Live 12 Suite" = "C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe"
  "Bitwig Studio" = "C:\Program Files\Bitwig Studio\Bitwig Studio.exe"
}
foreach ($entry in $hosts.GetEnumerator()) {
  if (Test-Path $entry.Value) {
    "INSTALLED`t$($entry.Key)`t$($entry.Value)"
    (Get-Item $entry.Value).VersionInfo | Select-Object ProductVersion,FileVersion
  } else {
    "MISSING`t$($entry.Key)`t$($entry.Value)"
  }
}
```

Expected in the current environment:

```text
INSTALLED    Reaper x64    C:\Program Files\REAPER (x64)\reaper.exe
MISSING      FL Studio 21  C:\Program Files\Image-Line\FL Studio 21\FL64.exe
MISSING      Cubase 13     C:\Program Files\Steinberg\Cubase 13\Cubase13.exe
MISSING      Studio One 6  C:\Program Files\PreSonus\Studio One 6\Studio One.exe
MISSING      Ableton Live 12 Suite C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe
MISSING      Bitwig Studio C:\Program Files\Bitwig Studio\Bitwig Studio.exe
```

If a host is missing, record `BLOCKED - host not installed at checked path` in the matrix. Do not mark tasks `11.3` or `12.4` complete until every required host has either been validated or the OpenSpec scope is explicitly changed.

- [ ] **Step 2: Validate Reaper interactive UI/export cells**

In REAPER v7.71/x64 with the current real bundle:

1. Add `VST3: ACE-Step (Allwave Media)` to a track.
2. Open the editor and visually verify layout is usable.
3. Click capture arm and clear controls; confirm controls respond and do not interrupt audio pass-through.
4. Confirm generation UI state is idle and model setup status is truthful.
5. Trigger WAV Save As for an existing generated WAV asset if one is available.
6. Drag a generated WAV asset to the arrange view if one is available.
7. Confirm MIDI export is visibly gated unavailable when no reliable note/onset data exists.
8. Confirm stem export is visibly gated unavailable unless successful stem WAV outputs exist.
9. Save, list, load, rename, and delete a preset; confirm loading does not auto-start generation.

- [ ] **Step 3: Record Reaper matrix result**

Update the Reaper row in `ACE-Step-Plugin\docs\host-compatibility-matrix.md` with observed `PASS`, `FAIL`, or `BLOCKED` cells. Use this exact note block after the current Reaper validation record:

```markdown
### Current real-bundle Reaper interactive UI/export check

| Field | Value |
|---|---|
| Host | REAPER v7.71/x64 |
| Host executable | `C:\Program Files\REAPER (x64)\reaper.exe` |
| Build commit | output of `git rev-parse --short=12 HEAD` |
| Bundle path | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | `allwavemedia` |
| Result | PASS |

Manual results:
- Editor layout: PASS
- Capture controls: PASS
- Generation UI state: PASS
- WAV Save As: PASS
- WAV drag/drop: PASS
- MIDI gated unavailable state: PASS
- Stem gated unavailable state: PASS
- Preset browsing: PASS
- Host-owned differences: record observed Reaper import prompts, insertion point, and media naming behavior.
```

If a generated WAV asset is not available, run a real-engine generation or record the export cells as `BLOCKED - no generated WAV asset available in this validation run`.

- [ ] **Step 4: Validate remaining installed hosts**

For each installed host among FL Studio, Cubase, Studio One, Ableton Live, and Bitwig:

1. Scan the real bundle path.
2. Load ACE-Step on a track/channel.
3. Confirm editor opens.
4. Confirm pass-through with a non-silent signal.
5. Confirm capture controls respond.
6. Confirm generation UI idle/model state is truthful.
7. Validate WAV Save As.
8. Validate WAV drag/drop or record Save As fallback when host-owned behavior differs.
9. Confirm MIDI/stem gated unavailable states.
10. Validate preset browse/save/load/rename/delete.

Record one section per host using this shape:

```markdown
### Current real-bundle <Host Name> validation

| Field | Value |
|---|---|
| Host executable | path printed by Step 1 |
| Host version | ProductVersion/FileVersion printed by Step 1 or value from About dialog |
| Build commit | output of `git rev-parse --short=12 HEAD` |
| Bundle path | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | `allwavemedia` |
| Result | PASS |

Manual results:
- Scan/load: PASS
- Pass-through: PASS
- Editor layout: PASS
- Capture controls: PASS
- Generation UI state: PASS
- WAV Save As: PASS
- WAV drag/drop: PASS or PASS WITH SAVE AS FALLBACK
- MIDI gated unavailable state: PASS
- Stem gated unavailable state: PASS
- Preset browsing: PASS
- Host-owned differences: record import prompts, insertion location, and project media copy behavior.
```

- [ ] **Step 5: Mark tasks 11.3 and 12.4 complete only when the full host matrix is validated**

Edit `openspec\changes\add-juce-vst3-plugin\tasks.md`:

```markdown
- [x] 11.3 Verify scan/load, pass-through, editor layout, capture controls, generation UI state, WAV export, MIDI export when available, stem export, and preset browsing across Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig.
- [x] 12.4 Verify external drag-and-drop in Reaper, FL Studio, Cubase, Studio One, Ableton Live, and Bitwig, with Save As as fallback.
```

Do not mark either task complete if any required host remains missing or unvalidated.

- [ ] **Step 6: Validate and commit DAW matrix evidence**

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): complete DAW host matrix validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

---

## Task 4: Complete OpenSpec 12.5 Error-State Validation

**Files:**
- Modify: `ACE-Step-Plugin\docs\host-compatibility-matrix.md`
- Modify: `openspec\changes\add-juce-vst3-plugin\tasks.md`

- [ ] **Step 1: Prepare safe error-state validation**

```powershell
$modelRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models"
$backupRoot = Join-Path $env:LOCALAPPDATA "AceStepPlugin\models.validation-backup"
Test-Path $modelRoot
git rev-parse --short=12 HEAD
```

Expected:

```text
True
```

Before moving or corrupting any model file, make a backup:

```powershell
if (Test-Path $backupRoot) { Remove-Item -Recurse -Force $backupRoot }
Copy-Item -Recurse $modelRoot $backupRoot
```

- [ ] **Step 2: Validate missing-model error**

1. Move one required model file out of `%LOCALAPPDATA%\AceStepPlugin\models`.
2. Launch the plugin in Reaper.
3. Open the editor.
4. Confirm the editor reports the missing model clearly and the host does not crash.
5. Restore the model file from backup.

Record:

```markdown
- Missing model: PASS - editor reported the missing model and Reaper remained stable.
```

- [ ] **Step 3: Validate checksum-mismatch error**

1. Copy one model file to a temporary file.
2. Corrupt the temporary validation copy by appending bytes.
3. Point validation at the corrupt copy only if the plugin supports selecting that path; otherwise record `BLOCKED - plugin UI does not support selecting an alternate model path for destructive checksum testing`.
4. Confirm checksum mismatch is surfaced without a host crash.
5. Restore all original models from backup.

Record:

```markdown
- Checksum mismatch: PASS or BLOCKED with exact reason.
```

- [ ] **Step 4: Validate backend-load failure**

1. Copy the real bundle to a temporary validation directory.
2. Remove or rename one backend DLL from the copied bundle, not from the canonical `C:\b\ace-ninja` bundle.
3. Scan/load the copied bundle in Reaper or AudioPluginHost.
4. Confirm the editor reports backend-load failure without crashing the host.
5. Delete the temporary copied bundle.

Record:

```markdown
- Backend-load failure: PASS - missing backend DLL error surfaced and host remained stable.
```

- [ ] **Step 5: Validate cancellation and generation failure**

1. Start a generation request with valid models.
2. Click cancel while generation is in progress.
3. Confirm cancellation status appears and the host remains responsive.
4. Start a generation request with an invalid output path or invalid request parameter if the UI exposes one.
5. Confirm generation failure status appears and the host remains responsive.

Record:

```markdown
- Cancellation: PASS - generation cancelled and host remained stable.
- Generation failure: PASS - failure surfaced in editor and host remained stable.
```

- [ ] **Step 6: Validate MIDI/stem/preset unavailable and failure states**

1. Confirm MIDI export unavailable state is visible for normal generated assets because reliable note/onset data is not available.
2. Confirm stem export unavailable state is visible unless successful stem WAV outputs exist.
3. Write an invalid JSON file into the preset directory.
4. Open the preset browser.
5. Confirm valid presets remain listed and the invalid file warning is surfaced.
6. Delete the invalid JSON file.

Record:

```markdown
- MIDI unavailable state: PASS
- Stem unavailable state: PASS
- Preset invalid JSON: PASS - valid presets remained listed and the warning was shown.
```

- [ ] **Step 7: Validate OOM only if safe**

Do not intentionally exhaust system memory on a shared workstation. If no safe bounded OOM trigger exists, record:

```markdown
- Out-of-memory: BLOCKED - no safe bounded OOM trigger was available in this shared Windows validation environment.
```

- [ ] **Step 8: Add the error-state validation record**

Append to `ACE-Step-Plugin\docs\host-compatibility-matrix.md`:

```markdown
## Error-state validation record

| Field | Value |
|---|---|
| Host | REAPER v7.71/x64 |
| Build commit | output of `git rev-parse --short=12 HEAD` |
| Bundle path | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | `allwavemedia` |
| Result | PASS except explicitly blocked unsafe OOM case |

Results:
- Missing model: PASS
- Checksum mismatch: PASS or BLOCKED with exact reason
- Backend-load failure: PASS
- Cancellation: PASS
- Generation failure: PASS
- MIDI unavailable state: PASS
- Stem unavailable state: PASS
- Preset invalid JSON: PASS
- Out-of-memory: BLOCKED - no safe bounded OOM trigger was available in this shared Windows validation environment.
```

- [ ] **Step 9: Mark task 12.5 complete only if all non-blocked states pass and blockers are accepted**

Edit `openspec\changes\add-juce-vst3-plugin\tasks.md`:

```markdown
- [x] 12.5 Verify missing model, checksum mismatch, out-of-memory, backend-load failure, cancellation, generation failure, MIDI unavailable state, stem failure, preset load failure, and host compatibility errors surface in the editor without crashing the host.
```

If checksum mismatch, OOM, or another state is blocked, keep task `12.5` unchecked unless the OpenSpec scope is explicitly updated to accept that blocker.

- [ ] **Step 10: Validate and commit error-state evidence**

```powershell
git --no-pager diff --check
openspec validate add-juce-vst3-plugin --strict
git add -- ACE-Step-Plugin\docs\host-compatibility-matrix.md openspec\changes\add-juce-vst3-plugin\tasks.md
git commit -m "docs(plugin): record editor error-state validation" -m "Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>"
git push origin feat/add-juce-vst3-plugin-8-3-onwards
```

---

## Task 5: Final PR Review, Merge, and Local Main Sync

**Files:**
- Modify only if PR body is stale: PR #2 description.

- [ ] **Step 1: Run final local validation**

```powershell
cmake -S "ACE-Step-Plugin" -B "ACE-Step-Plugin\build-codex-tests" -G "Visual Studio 17 2022" -A x64 -DACESTEP_ENABLE_ACESTEP_CPP=OFF -DACESTEP_BUILD_TESTS=ON
cmake --build "ACE-Step-Plugin\build-codex-tests" --config RelWithDebInfo --target AceStepPluginTests --parallel
ctest --test-dir "ACE-Step-Plugin\build-codex-tests" -C RelWithDebInfo --output-on-failure
.\ACE-Step-Plugin\scripts\build-stub-vst3.ps1
.\ACE-Step-Plugin\scripts\validate-bundle.ps1 -BuildDir C:\b\ace-ninja -Config RelWithDebInfo
openspec validate add-juce-vst3-plugin --strict
git --no-pager diff --check
```

Expected:

```text
Change 'add-juce-vst3-plugin' is valid
SUCCESS: Stub VST3 built successfully.
```

- [ ] **Step 2: Confirm no OpenSpec task remains unchecked**

```powershell
Select-String -Path "openspec\changes\add-juce-vst3-plugin\tasks.md" -Pattern "^- \[ \]"
```

Expected: no output. If there is output, do not merge.

- [ ] **Step 3: Request or verify final Copilot review**

```powershell
gh pr view 2 --repo allwavemedia/ACE-Step-1.5 --json reviewRequests,latestReviews,statusCheckRollup,mergeStateStatus,mergeable,isDraft
```

If Copilot review has not run against the final head commit, request it:

```powershell
gh pr review 2 --repo allwavemedia/ACE-Step-1.5 --comment --body "Ready for final automated review after completing evidence-gated validation."
```

Resolve any new review comments using `receiving-code-review`.

- [ ] **Step 4: Merge PR #2 only when all merge gates pass**

Confirm all of these are true:

```text
OpenSpec has no unchecked tasks.
All non-outdated review threads are resolved.
PR is not draft.
PR merge state is CLEAN.
Required checks are successful or absent because this repository has no required CI.
```

Merge:

```powershell
gh pr merge 2 --repo allwavemedia/ACE-Step-1.5 --squash --delete-branch
```

If GitHub prompts for a commit message, use:

```text
feat(plugin): add JUCE VST3 plugin implementation and validation

Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>
```

- [ ] **Step 5: Sync local main after merge**

```powershell
git fetch origin
git switch main
git pull --ff-only origin main
git --no-pager log --oneline -5
```

Expected: local `main` includes the merge commit from PR #2.

---

## Self-Review Notes

- Spec coverage: remaining gates map to tasks `2.7`, `11.3`, `12.4`, and `12.5`; final PR merge maps to Task 5.
- Placeholder scan: this plan avoids `TBD` and `TODO`; unknown validation outputs are captured by commands or recorded as explicit blockers.
- Type consistency: this plan does not introduce production types or APIs; it modifies validation docs and OpenSpec state only.
