# Add VST3 Functional Parity

## Why

The current ACE-Step VST3 validates as a loadable pass-through plugin, but it does not meet the application purpose of generating music in a DAW. The live editor lacks prompt-to-WAV generation controls, cancellation, generated asset history, WAV export, MIDI export, stem export, and integrated model/tool setup for all required local assets.

## What Changes

- Add a single-scroll VST3 editor workflow for setup, generation, capture, progress, generated assets, exports, presets, and diagnostics.
- Add plugin-safe background orchestration for ACE-Step generation, MIDI transcription, and stem separation.
- Use local sidecars where needed to keep host scan/load and the audio callback stable.
- Make WAV, MIDI, and stem export functional.
- Replace “MIDI/stem unavailable for v1” assumptions with researched local implementations.
- Add real destructive validation for model, sidecar, generation, MIDI, stem, preset, and host error states.

## Impact

- Supersedes incomplete functional claims in `add-juce-vst3-plugin`.
- Adds local tool/model dependencies selected by technical research.
- Requires expanded host validation after implementation.
