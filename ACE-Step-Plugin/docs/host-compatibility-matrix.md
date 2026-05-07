# Host Compatibility Matrix

This matrix defines the ACE-Step VST3 v1 host validation target and separates
plugin-owned behavior from host-owned differences.

## Scope

The supported v1 validation matrix is Windows VST3 in these hosts:

| Host | Scan/load | Pass-through | Editor layout | Capture controls | Generation UI state | WAV Save As | WAV drag/drop | MIDI gated unavailable state | Stem gated unavailable state | Preset browsing | Host-owned differences |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Reaper | PASS (2026-05-07 current PR-head) | PASS (2026-05-07 current PR-head, peak_diff=0.000000000) | Partial: FX UI opens; full visual layout pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Timeline insertion point, media item naming, import prompts |
| AudioPluginHost | BLOCKED — vendored JUCE incomplete; cannot build host locally | BLOCKED | BLOCKED | BLOCKED | BLOCKED | BLOCKED | BLOCKED | BLOCKED | BLOCKED | BLOCKED | N/A |
| FL Studio | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | Browser/drop target behavior, channel rack vs playlist placement |
| Cubase | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | Pool import prompts, project media copy policy |
| Studio One | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | Browser/import prompts, arrange view insertion behavior |
| Ableton Live | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | Audio clip placement, browser import prompts |
| Bitwig | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | BLOCKED - host not installed at checked path | Launcher/arranger drop interpretation, import prompts |

Do not convert any pending cell to pass/fail without recording the exact host
version, bundle path, build commit, tester, and result.

## Current validation status

Validation run:

| Field | Value |
|---|---|
| Build commit | `f58ab9e75975` (docs commits on top) |
| OS | Microsoft Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Real bundle | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Bundle DLL timestamp | 2026-05-06 20:59 |
| Result | Reaper scan/load and pass-through PASS (2026-05-07). AudioPluginHost blocked by incomplete vendored JUCE tree. All other hosts not installed. |

| Host | Checked executable path | Result |
|---|---|---|
| Reaper x64 | `C:\Program Files\REAPER (x64)\reaper.exe` | Installed; current PR-head scan/load and pass-through PASS (2026-05-07) |
| AudioPluginHost | (built from vendored JUCE) | BLOCKED — vendored JUCE `extras/Build/` and `harfbuzz` sources missing |
| FL Studio 21 | `C:\Program Files\Image-Line\FL Studio 21\FL64.exe` | BLOCKED - host not installed |
| Cubase 13 | `C:\Program Files\Steinberg\Cubase 13\Cubase13.exe` | BLOCKED - host not installed |
| Studio One 6 | `C:\Program Files\PreSonus\Studio One 6\Studio One.exe` | BLOCKED - host not installed |
| Ableton Live 12 Suite | `C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe` | BLOCKED - host not installed |
| Bitwig Studio | `C:\Program Files\Bitwig Studio\Bitwig Studio.exe` | BLOCKED - host not installed |

## JUCE AudioPluginHost validation record

**Status:** BLOCKED — vendored JUCE 8.0.10 tree missing `extras/Build/` CMake infrastructure and
incomplete harfbuzz source. AudioPluginHost cannot be built from the vendored tree.

See `ACE-Step-Plugin\docs\audiopluginhost-blocker-investigation.md` for full details and required
resolution steps.

## Reaper validation record

### Current PR-head automated check (2026-05-07)

| Field | Value |
|---|---|
| Host | REAPER v7.71/x64 |
| Host executable | `C:\Program Files\REAPER (x64)\reaper.exe` |
| OS | Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Build commit | HEAD (docs commits on top of `f58ab9e75975`) |
| Bundle path | `C:\b\ace-ninja\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Bundle DLL timestamp | 2026-05-06 20:59 |
| Tester | Copilot CLI automated ReaScript validation |
| Evidence artifacts | `C:\Users\ldoby\.copilot\session-state\17c40baf-3155-4588-a905-9749f6291ef7\files\reaper-validation-current\` |

**Scan/load:** PASS — `EnumInstalledFX` found `VST3: ACE-Step (Allwave Media)` at index 3 within
333 ms; `TrackFX_AddByName` inserted with FX index 0; `TrackFX_GetOpen` confirmed UI open at 448 ms.

**Offline pass-through:** PASS — 48 kHz stereo 440 Hz sine WAV, ACE-Step bypassed then enabled:
`baseline_peak=0.249969482`, `enabled_peak=0.249969482`, `peak_diff=0.000000000`,
`baseline_rms=0.176757252`, `enabled_rms=0.176757252`, `rms_diff=0.000000000`.

**Stability:** PASS — Reaper process remained live throughout both validation scripts.

### Prior automated check (commit `a17af3ea`, 2026-05-06)

| Field | Value |
|---|---|
| Build commit | `a17af3ea` |
| Bundle DLL size | 13,638,144 bytes |
| Bundle timestamp | 2026-05-06 21:51 |
| Evidence artifacts | `C:\Users\ldoby\.copilot\session-state\4eeaba01-8b77-4fe6-8bdc-8eb1c614ce76\files\reaper-validation-current-real-fast\` |

- Scan/load: PASS. FX UI: PASS. Offline pass-through: PASS (`peak_diff=0.000000000`, `rms_diff=0.000000000`).
- Scan-time signal: ACE-Step found within 318 ms; no GGUF models loaded during scan.

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
