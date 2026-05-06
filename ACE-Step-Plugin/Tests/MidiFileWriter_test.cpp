#include "../Source/MIDI/MidiFileWriter.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <vector>

namespace acestep_plugin
{

class MidiFileWriterTests final : public juce::UnitTest
{
public:
    MidiFileWriterTests() : juce::UnitTest("MidiFileWriter") {}

    void runTest() override
    {
        beginTest("writes valid midi file for explicit notes");
        {
            const auto tempFile = juce::File::createTempFile(".mid");
            tempFile.deleteFile();

            const std::vector<MidiNoteEvent> notes {
                { 60, 0.0, 0.5, 96 },
                { 64, 0.5, 0.5, 88 },
            };

            const auto result = MidiFileWriter::writeType0(tempFile, notes, 120.0);

            expect(result.success, result.errorMessage);
            expect(tempFile.existsAsFile());
            expectGreaterThan(static_cast<int>(tempFile.getSize()), 14);

            juce::FileInputStream input(tempFile);
            expect(input.openedOk());

            int midiFileType = -1;
            juce::MidiFile midiFile;
            expect(midiFile.readFrom(input, true, &midiFileType));
            expectEquals(midiFileType, 0);
            expectEquals(midiFile.getNumTracks(), 1);
            expectEquals(static_cast<int>(midiFile.getTimeFormat()), 960);

            const auto* track = midiFile.getTrack(0);
            expect(track != nullptr);
            expect(track->getNumEvents() >= 4);

            const auto* firstEvent = track->getEventPointer(0);
            expect(firstEvent->message.isNoteOn());
            expectEquals(firstEvent->message.getNoteNumber(), 60);
            expectEquals(static_cast<int>(firstEvent->message.getTimeStamp()), 0);

            const auto* secondEvent = track->getEventPointer(1);
            expect(secondEvent->message.isNoteOff());
            expectEquals(secondEvent->message.getNoteNumber(), 60);
            expectEquals(static_cast<int>(secondEvent->message.getTimeStamp()), 960);

            tempFile.deleteFile();
        }

        beginTest("does not create midi file for unavailable note data");
        {
            const auto tempFile = juce::File::createTempFile(".mid");
            tempFile.deleteFile();

            const auto result = MidiFileWriter::writeType0(tempFile, {}, 120.0);

            expect(!result.success);
            expect(result.errorMessage.containsIgnoreCase("no midi notes"));
            expect(!tempFile.existsAsFile());
        }

        beginTest("rejects invalid pitch and duration");
        {
            const auto tempFile = juce::File::createTempFile(".mid");
            tempFile.deleteFile();

            const std::vector<MidiNoteEvent> notes {
                { 128, 0.0, 0.5, 96 },
            };

            const auto result = MidiFileWriter::writeType0(tempFile, notes, 120.0);

            expect(!result.success);
            expect(result.errorMessage.containsIgnoreCase("pitch"));
            expect(!tempFile.existsAsFile());
        }

        beginTest("sorts notes by onset before writing");
        {
            const auto tempFile = juce::File::createTempFile(".mid");
            tempFile.deleteFile();

            const std::vector<MidiNoteEvent> notes {
                { 67, 1.0, 0.25, 80 },
                { 60, 0.0, 0.25, 80 },
            };

            const auto result = MidiFileWriter::writeType0(tempFile, notes, 120.0);
            expect(result.success, result.errorMessage);

            juce::FileInputStream input(tempFile);
            juce::MidiFile midiFile;
            expect(midiFile.readFrom(input));

            const auto* track = midiFile.getTrack(0);
            expect(track != nullptr);
            expectEquals(track->getEventPointer(0)->message.getNoteNumber(), 60);

            tempFile.deleteFile();
        }

        beginTest("creates parent directory for midi export path");
        {
            const auto exportDirectory =
                juce::File::getSpecialLocation(juce::File::tempDirectory)
                    .getChildFile("acestep-midi-writer-tests")
                    .getChildFile(juce::Uuid().toString());
            const auto tempFile = exportDirectory.getChildFile("generated.mid");

            const std::vector<MidiNoteEvent> notes {
                { 60, 0.0, 0.25, 96 },
            };

            const auto result = MidiFileWriter::writeType0(tempFile, notes, 120.0);

            expect(result.success, result.errorMessage);
            expect(exportDirectory.isDirectory());
            expect(tempFile.existsAsFile());

            exportDirectory.deleteRecursively();
        }
    }
};

static MidiFileWriterTests sMidiFileWriterTests;

} // namespace acestep_plugin
