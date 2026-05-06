#include "MidiFileWriter.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <algorithm>
#include <cmath>

namespace acestep_plugin
{
namespace
{

bool isValid(const MidiNoteEvent& note) noexcept
{
    return note.pitch >= 0 && note.pitch <= 127
        && note.velocity >= 1 && note.velocity <= 127
        && std::isfinite(note.startSeconds)
        && std::isfinite(note.durationSeconds)
        && note.startSeconds >= 0.0
        && note.durationSeconds > 0.0;
}

double secondsToTicks(double seconds, double bpm) noexcept
{
    const auto quartersPerSecond = bpm / 60.0;
    return seconds * quartersPerSecond
        * static_cast<double>(MidiFileWriter::defaultTicksPerQuarterNote);
}

} // namespace

MidiFileWriteResult MidiFileWriter::writeType0(
    const juce::File& destination,
    const std::vector<MidiNoteEvent>& notes,
    double bpm)
{
    if (notes.empty())
        return { false, "No MIDI notes are available for this asset." };

    if (!std::isfinite(bpm) || bpm <= 0.0)
        return { false, "MIDI tempo must be greater than zero." };

    for (const auto& note : notes)
        if (!isValid(note))
            return { false, "Invalid MIDI note pitch, velocity, start time, or duration." };

    auto sortedNotes = notes;
    std::sort(sortedNotes.begin(), sortedNotes.end(), [](const auto& left, const auto& right) {
        return left.startSeconds < right.startSeconds;
    });

    juce::MidiMessageSequence sequence;

    for (const auto& note : sortedNotes)
    {
        const auto startTick = secondsToTicks(note.startSeconds, bpm);
        const auto endTick = secondsToTicks(note.startSeconds + note.durationSeconds, bpm);

        sequence.addEvent(
            juce::MidiMessage::noteOn(1, note.pitch, static_cast<juce::uint8>(note.velocity)),
            startTick);
        sequence.addEvent(juce::MidiMessage::noteOff(1, note.pitch), endTick);
    }

    sequence.updateMatchedPairs();

    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(defaultTicksPerQuarterNote);
    midiFile.addTrack(sequence);

    if (!destination.getParentDirectory().createDirectory())
        return { false, "Could not create MIDI destination directory." };

    juce::FileOutputStream output(destination);
    if (!output.openedOk())
        return { false, "Could not open MIDI destination for writing." };

    output.setPosition(0);
    if (output.truncate().failed())
        return { false, "Could not replace existing MIDI destination." };

    if (!midiFile.writeTo(output, 0))
        return { false, "Could not write MIDI file." };

    output.flush();
    return { true, {} };
}

} // namespace acestep_plugin
