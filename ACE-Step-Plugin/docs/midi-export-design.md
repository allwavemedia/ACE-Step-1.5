# MIDI Export Design

## Background

Task 8.1 investigated whether `acestep.cpp` exposes reliable note/event data for MIDI
extraction. This document records that finding and the resulting design for tasks 8.3–8.5.

## Backend Capability Finding (Task 8.1)

The vendored `ServeurpersoCom/acestep.cpp` backend exposes audio generation through
`acestep_generate_wav` (see `External/acestep_cpp/src/acestep_capi.h`). Its request and
response surface carries:

- Generated `audio_codes` — FSQ (Finite Scalar Quantization) discrete tokens at approximately
  5 Hz, representing the compressed latent space of the audio model.
- WAV file output — decoded audio at 44.1 kHz.

The backend does **not** expose note events, onset times, pitch values, or any MIDI-compatible
event stream. The FSQ `audio_codes` are not musically interpretable tokens and cannot be
trivially mapped to MIDI note data.

**Conclusion:** Plugin MIDI export must remain unavailable (gated via `MidiExportAvailability::unavailable`)
until a post-processing AMT (Automatic Music Transcription) path is designed and integrated.

## Availability Gating (Task 8.2)

`GeneratedAsset` carries a `MidiExportAvailability` field (defined in
`Source/Models/GeneratedAssetHistory.h`):

```cpp
enum class MidiExportAvailability { unavailable, available };
```

All assets default to `MidiExportAvailability::unavailable`. `GeneratedAssetTile` renders the
MIDI export button as disabled and labelled "MIDI N/A" whenever `canExportMidi()` returns
`false`. This ensures the export path is exercisable in the UI without exposing broken
functionality.

## Planned Implementation Path (Tasks 8.3–8.5)

When an AMT path becomes available, the implementation steps are:

1. **8.3 — MIDI file writer**: Implement a `MidiFileWriter` service in
   `Source/Services/MidiFileWriter.h/.cpp` that accepts a note-event list and writes a
   standards-compliant SMF Type 0 (single-track) `.mid` file. Use `juce::MidiFile` and
   `juce::MidiMessageSequence`.

2. **8.4 — Export paths**: Wire MIDI drag-and-drop via `performExternalDragDropOfFiles` and
   a "Save As" dialog using `juce::FileChooser`. Follow the same copy-semantics pattern as
   WAV export in `GeneratedAssetTile`.

3. **8.5 — Tests**: Add unit tests for `MidiFileWriter` (valid output, empty sequence, error
   path), `GeneratedAssetTile` MIDI button states, and the full gate→write→export path.

## Future AMT Integration Note

A suitable AMT path could be:
- A bundled lightweight pitch-detection library (e.g., CREPE ONNX, BasicPitch) applied as a
  post-processing step to the generated WAV.
- An external sidecar process called from the plugin worker thread after WAV generation.
- A future `acestep.cpp` API extension that exposes symbolic music output.

Any of these paths would set `midiAvailability = MidiExportAvailability::available` on the
resulting `GeneratedAsset`, unlocking the MIDI button in the tile UI without any further UI
changes.
