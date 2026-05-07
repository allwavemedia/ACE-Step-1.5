# Host Compatibility Matrix

This matrix defines the ACE-Step VST3 v1 host validation target and separates
plugin-owned behavior from host-owned differences.

## Scope

The supported v1 validation matrix is Windows VST3 in these hosts:

| Host | Scan/load | Pass-through | Editor layout | Capture controls | Generation UI state | WAV Save As | WAV drag/drop | MIDI gated unavailable state | Stem gated unavailable state | Preset browsing | Host-owned differences |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Reaper | Current real-bundle automated PASS | Current real-bundle automated PASS | Partial: current evidence shows FX UI opens; full visual layout pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Timeline insertion point, media item naming, import prompts |
| FL Studio | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Browser/drop target behavior, channel rack vs playlist placement |
| Cubase | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pool import prompts, project media copy policy |
| Studio One | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Browser/import prompts, arrange view insertion behavior |
| Ableton Live | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Audio clip placement, browser import prompts |
| Bitwig | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Launcher/arranger drop interpretation, import prompts |

Do not convert any pending cell to pass/fail without recording the exact host
version, bundle path, build commit, tester, and result.

## Current validation status

Validation run:

| Field | Value |
|---|---|
| Build commit | `a17af3ea` |
| OS | Microsoft Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Real bundle | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Bundle DLL size | 13,638,144 bytes |
| Bundle DLL timestamp | 2026-05-06 21:51 |
| Result | Real bundle validation passed for CPU/CUDA/Vulkan GGML sibling DLLs. Current real-bundle Reaper automation passed scan/load, FX UI-open, and offline pass-through. Current AudioPluginHost generated-graph launch opened the ACE-Step editor, but AudioPluginHost pass-through remains pending. Full DAW matrix validation remains pending for FL Studio, Cubase, Studio One, Ableton Live, and Bitwig, which are not installed in this environment. |

The CLI validation-prep environment can build the stub VST3 bundle and can run
isolated Reaper automation. Full evidence-gated manual host validation remains
pending because most target hosts are not installed and several Reaper cells
still require interactive UI/export checks.

| Host | Checked executable path | Result |
|---|---|---|
| Reaper x64 | `C:\Program Files\REAPER (x64)\reaper.exe` | Installed; current real-bundle automated scan/load and pass-through evidence recorded below |
| Reaper | `C:\Program Files\REAPER\reaper.exe` | Missing |
| FL Studio 21 | `C:\Program Files\Image-Line\FL Studio 21\FL64.exe` | Missing |
| Cubase 13 | `C:\Program Files\Steinberg\Cubase 13\Cubase13.exe` | Missing |
| Studio One 6 | `C:\Program Files\PreSonus\Studio One 6\Studio One.exe` | Missing |
| Ableton Live 12 Suite | `C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe` | Missing |
| Bitwig Studio | `C:\Program Files\Bitwig Studio\Bitwig Studio.exe` | Missing |

The Reaper automation below is valid evidence for current real-bundle scan/load,
FX UI-open, and pass-through only. It is not a complete pass/fail result for
Task 11.3 or 12.4 because interactive UI/export checks and the other target
hosts are still pending.

## JUCE AudioPluginHost validation record

### Current real-bundle generated-graph load/editor check

| Field | Value |
|---|---|
| Host executable | `ACE-Step-Plugin\External\JUCE\build-audio-plugin-host\extras\AudioPluginHost\AudioPluginHost_artefacts\RelWithDebInfo\AudioPluginHost.exe` |
| OS | Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Bundle path | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | Copilot CLI generated-graph launch validation |
| Evidence artifacts | `C:\Users\ldoby\.copilot\session-state\4eeaba01-8b77-4fe6-8bdc-8eb1c614ce76\files\audiopluginhost-validation-current-real\` |

**Current validation status:**

- **Load/editor:** PASS. AudioPluginHost launched a generated
  `.filtergraph` containing the current real-bundle ACE-Step VST3, and the
  process main window title was `ACE-Step (VST3)`.
- **Stability:** PARTIAL PASS. The host stayed alive for evidence capture and
  was stopped cleanly by exact PID.
- **Pass-through:** PENDING. AudioPluginHost audio routing and level-matched
  pass-through have not yet been measured.

## Reaper validation record

### Current real-bundle automated check (a17af3ea)

| Field | Value |
|---|---|
| Host | REAPER v7.71/x64 |
| Host executable | `C:\Program Files\REAPER (x64)\reaper.exe` |
| OS | Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Build commit | `a17af3ea` |
| Bundle path | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Bundle DLL size | 13,638,144 bytes |
| Bundle timestamp | 2026-05-06 21:51 |
| Tester | Copilot CLI automated ReaScript validation |
| Evidence artifacts | `C:\Users\ldoby\.copilot\session-state\4eeaba01-8b77-4fe6-8bdc-8eb1c614ce76\files\reaper-validation-current-real-fast\` |

**Current validation status:**

- **Scan/load:** PASS. `EnumInstalledFX` found
  `VST3: ACE-Step (Allwave Media)` at index `3`, and
  `TrackFX_AddByName` inserted the current real bundle with FX index `0`.
- **FX UI:** PASS. `TrackFX_GetOpen` reported the FX UI opened.
- **Offline pass-through:** PASS. A 48 kHz stereo sine WAV was measured with
  ACE-Step bypassed and enabled; peak/RMS values were identical:
  `baseline_peak=0.250000000`, `enabled_peak=0.250000000`,
  `peak_diff=0.000000000`, `baseline_rms=0.176771017`,
  `enabled_rms=0.176771017`, `rms_diff=0.000000000`.
- **Scan-time signal:** The probe found the plugin within `318 ms` of script
  start and the pass-through probe found it within `182 ms`; no GGUF models
  were loaded during construction or scan. After all four GGUF model files were
  installed under `%LOCALAPPDATA%\AceStepPlugin\models`, a fresh isolated Reaper
  profile found the same real bundle within `199 ms` of ReaScript start
  (`wall_ms=17105`, including REAPER startup wait).

### Prior automated validation evidence (7909e8460d88)

| Field | Value |
|---|---|
| Build commit | `7909e8460d88` |
| Tester | Copilot CLI automated ReaScript validation |
| Evidence artifacts | `C:\Users\ldoby\.copilot\session-state\4eeaba01-8b77-4fe6-8bdc-8eb1c614ce76\files\reaper-validation-ace\` |

Prior automated results:

- Scan/load and FX UI open: PASS. ReaScript `EnumInstalledFX` found
  `VST3: ACE-Step (Allwave Media)`, `TrackFX_AddByName` inserted it on a track
  with FX index `0`, and `TrackFX_GetOpen` reported the FX UI open.
- Offline pass-through: PASS. A 48 kHz stereo sine WAV was inserted on a Reaper
  track and measured with `CreateTrackAudioAccessor` /
  `GetAudioAccessorSamples` with ACE-Step bypassed and enabled. The measured
  peak/RMS values were identical:
  `baseline_peak=0.250000000`, `enabled_peak=0.250000000`,
  `peak_diff=0.000000000`, `baseline_rms=0.176771017`,
  `enabled_rms=0.176771017`, `rms_diff=0.000000000`.

**Task 2.7 status:** Reaper current real-bundle scan/load, FX UI-open, and
pass-through pass. AudioPluginHost validation remains pending. Task 2.7 cannot
be marked complete until both hosts pass with current evidence.

Remaining Reaper checks that still require interactive/manual validation:
editor layout details, capture controls, generation UI state, WAV Save As, WAV
drag/drop, MIDI unavailable state, stem unavailable state, preset browsing, and
host-specific drag/drop fallback behavior.

## Plugin-owned behavior

The plugin owns these behaviors and should keep them consistent across hosts:

- VST3 metadata: vendor `Allwave Media`, product `ACE-Step`.
- Fast scan/load path: construction must not load GGUF models or start generation.
- Audio pass-through: `processBlock` forwards input to output without generation work.
- Capture controls and meter state are editor-owned and must not block audio.
- Save As is the reliable fallback for generated WAV, MIDI when available, stem WAV,
  and future preset-backed assets.
- External drag starts copy-style file transfer; the plugin never asks the host to
  move or delete generated files.
- MIDI export remains unavailable unless the asset carries reliable note/event data.
- Stem export remains unavailable unless successful stem WAV outputs are present.
- Preset loading updates editor/model state only; it must not submit generation.

## Host-owned behavior

The host owns these differences, so validation records should note them without
treating them as plugin regressions:

- Plug-in scan UX, cache clearing, quarantine dialogs, and error log locations.
- External drag insertion location, clip naming, media-copy prompts, and whether the
  file lands in an arranger, browser, pool, or launcher area.
- Whether imported files are copied into a project media directory.
- File import prompts shown before accepting WAV, MIDI, or stem drag/drop.
- Focus, keyboard shortcuts, and exact window chrome around the plugin editor.

When drag/drop differs from the common path, use **Save As** as the documented
fallback and record the host-specific behavior in the validation notes.
