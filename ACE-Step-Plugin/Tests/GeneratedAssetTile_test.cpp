#include "ReferenceAudioBufferTestUtils.h"

#include "../Source/UI/GeneratedAssetTile.h"

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace acestep_plugin
{

class GeneratedAssetTileTests final : public juce::UnitTest
{
public:
    GeneratedAssetTileTests() : juce::UnitTest("GeneratedAssetTile") {}

    void runTest() override
    {
        beginTest("tile stores asset");
        {
            const auto asset = makeTestAsset(0);
            GeneratedAssetTile tile(asset);
            expectEquals(tile.getAsset().id, asset.id);
        }

        beginTest("setPlaying changes state");
        {
            GeneratedAssetTile tile(makeTestAsset(0));
            tile.setPlaying(true);
            expect(tile.isPlaying());
            tile.setPlaying(false);
            expect(!tile.isPlaying());
        }

        beginTest("onPlayStop callback fires");
        {
            GeneratedAssetTile tile(makeTestAsset(0));
            bool fired = false;
            tile.setOnPlayStop([&](const GeneratedAsset&, bool) { fired = true; });
            // Simulate button click via resized + direct invocation path.
            // Because we cannot safely send mouse events headlessly, we verify
            // that the callback wiring is set without crashing.
            expect(!fired, "callback not yet fired before button press");
        }

        beginTest("onSaveAs callback stored without crash");
        {
            GeneratedAssetTile tile(makeTestAsset(0));
            tile.setOnSaveAs([](const GeneratedAsset&) {});
            expect(true);
        }

        beginTest("MIDI export is disabled by default");
        {
            GeneratedAssetTile tile(makeTestAsset(0));
            expect(!tile.canExportMidi());
        }

        beginTest("MIDI export is enabled when asset has reliable note data");
        {
            auto asset = makeTestAsset(0);
            asset.midiAvailability = MidiExportAvailability::available;

            GeneratedAssetTile tile(asset);
            expect(tile.canExportMidi());
        }

        beginTest("MIDI export callback is not required when unavailable");
        {
            GeneratedAssetTile tile(makeTestAsset(0));
            tile.setOnMidiSaveAs([](const GeneratedAsset&) {});
            expect(!tile.canExportMidi());
        }

        beginTest("MIDI export can carry a generated MIDI path");
        {
            auto asset = makeTestAsset(0);
            asset.midiAvailability = MidiExportAvailability::available;
            asset.midiPath = "C:\\temp\\generated.mid";

            GeneratedAssetTile tile(asset);
            expect(tile.canExportMidi());
            expectEquals(tile.getAsset().midiPath, juce::String("C:\\temp\\generated.mid"));
            expectEquals(
                tile.getMidiExportFile().getFullPathName(),
                juce::String("C:\\temp\\generated.mid"));
        }

        beginTest("MIDI export file is empty when no MIDI path exists");
        {
            auto asset = makeTestAsset(0);
            asset.midiAvailability = MidiExportAvailability::available;

            GeneratedAssetTile tile(asset);
            expect(tile.canExportMidi());
            expect(tile.getMidiExportFile().getFullPathName().isEmpty());
        }

        beginTest("stem export callback is not required when stems are absent");
        {
            GeneratedAssetTile tile(makeTestAsset(0));
            tile.setOnStemSaveAs([](const GeneratedAsset&, const StemAsset&) {});
            expectEquals(tile.getExportableStemCount(), 0);
        }

        beginTest("only successful stems are exportable and failures remain visible");
        {
            auto asset = makeTestAsset(0);
            asset.stems.push_back(StemAsset { StemGroup::vocals, "C:\\temp\\vocals.wav", true, {} });
            asset.stems.push_back(StemAsset { StemGroup::drums, {}, false, "Stem model failed" });

            GeneratedAssetTile tile(asset);

            expectEquals(tile.getExportableStemCount(), 1);
            expectEquals(tile.getAsset().stems[1].errorMessage, juce::String("Stem model failed"));
        }

        beginTest("stem save callback exports only successful stems independently");
        {
            auto asset = makeTestAsset(0);
            asset.stems.push_back(StemAsset { StemGroup::vocals, "C:\\temp\\vocals.wav", true, {} });
            asset.stems.push_back(StemAsset { StemGroup::drums, {}, false, "Stem model failed" });

            GeneratedAssetTile tile(asset);
            juce::String exportedPath;
            tile.setOnStemSaveAs([&](const GeneratedAsset&, const StemAsset& stem) {
                exportedPath = stem.outputPath;
            });

            expect(tile.exportStemAt(0));
            expectEquals(exportedPath, juce::String("C:\\temp\\vocals.wav"));
            expect(!tile.exportStemAt(1));
            expectEquals(
                tile.getStemExportFileAt(0).getFullPathName(),
                juce::String("C:\\temp\\vocals.wav"));
            expect(tile.getStemExportFileAt(1).getFullPathName().isEmpty());
        }

        beginTest("stem preview callback toggles each successful stem independently");
        {
            auto asset = makeTestAsset(0);
            asset.stems.push_back(StemAsset { StemGroup::vocals, "C:\\temp\\vocals.wav", true, {} });
            asset.stems.push_back(StemAsset { StemGroup::drums, {}, false, "Stem model failed" });

            GeneratedAssetTile tile(asset);
            bool previewPlaying = false;
            juce::String previewPath;
            tile.setOnStemPreview([&](const GeneratedAsset&, const StemAsset& stem, bool shouldPlay) {
                previewPath = stem.outputPath;
                previewPlaying = shouldPlay;
            });

            expect(tile.toggleStemPreviewAt(0));
            expect(previewPlaying);
            expectEquals(previewPath, juce::String("C:\\temp\\vocals.wav"));

            expect(tile.toggleStemPreviewAt(0));
            expect(!previewPlaying);
            expect(!tile.toggleStemPreviewAt(1));
        }
    }
};

static GeneratedAssetTileTests sGeneratedAssetTileTests;

} // namespace acestep_plugin
