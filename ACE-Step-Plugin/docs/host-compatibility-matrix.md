# Host Compatibility Matrix

This matrix defines the ACE-Step VST3 v1 host validation target and separates
plugin-owned behavior from host-owned differences.

## Scope

The supported v1 validation matrix is Windows VST3 in these hosts:

| Host | Scan/load | Pass-through | Editor layout | Capture controls | Generation UI state | WAV Save As | WAV drag/drop | MIDI gated unavailable state | Stem gated unavailable state | Preset browsing | Host-owned differences |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Reaper | PASS (automated ReaScript smoke, see record below) | PASS (offline accessor pass-through, see record below) | Partial: FX UI opens; full visual layout pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Timeline insertion point, media item naming, import prompts |
| FL Studio | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Browser/drop target behavior, channel rack vs playlist placement |
| Cubase | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pool import prompts, project media copy policy |
| Studio One | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Browser/import prompts, arrange view insertion behavior |
| Ableton Live | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Audio clip placement, browser import prompts |
| Bitwig | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Pending manual validation | Launcher/arranger drop interpretation, import prompts |

Do not convert any pending cell to pass/fail without recording the exact host
version, bundle path, build commit, tester, and result.

## Current validation status

The CLI validation-prep environment can build the stub VST3 bundle and can run
isolated Reaper automation. Full evidence-gated manual host validation remains
pending because most target hosts are not installed and several Reaper cells
still require interactive UI/export checks.

| Host | Checked executable path | Result |
|---|---|---|
| Reaper x64 | `C:\Program Files\REAPER (x64)\reaper.exe` | Installed; partial automated evidence recorded below |
| Reaper | `C:\Program Files\REAPER\reaper.exe` | Missing |
| FL Studio 21 | `C:\Program Files\Image-Line\FL Studio 21\FL64.exe` | Missing |
| Cubase 13 | `C:\Program Files\Steinberg\Cubase 13\Cubase13.exe` | Missing |
| Studio One 6 | `C:\Program Files\PreSonus\Studio One 6\Studio One.exe` | Missing |
| Ableton Live 12 Suite | `C:\ProgramData\Ableton\Live 12 Suite\Program\Ableton Live 12 Suite.exe` | Missing |
| Bitwig Studio | `C:\Program Files\Bitwig Studio\Bitwig Studio.exe` | Missing |

This blocker is not a pass/fail result for Task 11.3 or 12.4. Those tasks
remain pending until a tester records real host versions, bundle path, build
commit, pass/fail results, and notes from interactive DAW validation.

## Reaper partial validation record

| Field | Value |
|---|---|
| Host | REAPER v7.71/x64 |
| Host executable | `C:\Program Files\REAPER (x64)\reaper.exe` |
| OS | Windows 11 Pro Insider Preview 10.0.26300 build 26300 |
| Build commit | `7909e8460d88` |
| Bundle path | `ACE-Step-Plugin\build-vst3-stub\AceStepPlugin_artefacts\RelWithDebInfo\VST3\ACE-Step.vst3` |
| Tester | Copilot CLI automated ReaScript validation |
| Evidence artifacts | `C:\Users\ldoby\.copilot\session-state\4eeaba01-8b77-4fe6-8bdc-8eb1c614ce76\files\reaper-validation-ace\` |

Recorded results:

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
