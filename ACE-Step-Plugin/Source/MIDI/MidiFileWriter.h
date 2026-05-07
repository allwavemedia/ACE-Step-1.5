#pragma once

#include "MidiNoteEvent.h"

#include <juce_core/juce_core.h>

#include <vector>

namespace acestep_plugin
{

/** Result returned by MIDI file writing. */
struct MidiFileWriteResult
{
    /** Whether the MIDI file was written successfully. */
    bool success = false;

    /** User-readable failure reason when success is false. */
    juce::String errorMessage;
};

/** Writes explicit note events as standards-compliant SMF Type 0 MIDI files. */
class MidiFileWriter final
{
public:
    /** Default pulses per quarter note used for exported files. */
    static constexpr int defaultTicksPerQuarterNote = 960;

    /** Write a single-track Type 0 MIDI file to the destination path.
     *
     *  Args:
     *      destination: The .mid file to create or replace.
     *      notes: Explicit note events to encode.
     *      bpm: Positive tempo used to convert note seconds to MIDI ticks.
     *
     *  Returns:
     *      A result containing success state and any failure reason.
     */
    static MidiFileWriteResult writeType0(
        const juce::File& destination,
        const std::vector<MidiNoteEvent>& notes,
        double bpm);

private:
    MidiFileWriter() = delete;
};

} // namespace acestep_plugin
