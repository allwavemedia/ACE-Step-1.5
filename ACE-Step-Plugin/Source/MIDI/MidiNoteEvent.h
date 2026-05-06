#pragma once

namespace acestep_plugin
{

/** A single explicit MIDI note event derived from reliable note/onset data. */
struct MidiNoteEvent
{
    /** MIDI pitch in the inclusive range 0..127. */
    int pitch = 60;

    /** Note start time in seconds from the beginning of the asset. */
    double startSeconds = 0.0;

    /** Note length in seconds. */
    double durationSeconds = 0.0;

    /** Note-on velocity in the inclusive MIDI range 1..127. */
    int velocity = 96;
};

} // namespace acestep_plugin
