#include "../Source/Presets/PresetStore.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class PresetStoreTests final : public juce::UnitTest
{
public:
    PresetStoreTests() : juce::UnitTest("PresetStore") {}

    void runTest() override
    {
        beginTest("saves and loads versioned generation preset JSON");
        {
            const auto directory = uniquePresetDirectory("acestep-presets-save-load");
            expect(directory.isDirectory(), "precondition: created preset directory");
            PresetStore store(directory);

            auto preset = makePreset();
            const auto saveResult = store.save(preset);
            expect(saveResult.success, saveResult.errorMessage);

            const auto loaded = store.load("ambient-sketch");
            expect(loaded.success, loaded.errorMessage);
            expectEquals(loaded.preset.name, juce::String("Ambient sketch"));
            expectEquals(loaded.preset.request.prompt, juce::String("warm ambient pads"));
            expectEquals(loaded.preset.schemaVersion, 1);
            expectEquals(loaded.preset.request.requestedStemGroups.size(), static_cast<size_t>(2));
            expect(loaded.preset.midiExportRequested);

            const auto listed = store.list();
            expect(listed.success, listed.errorMessage);
            expectEquals(static_cast<int>(listed.presets.size()), 1);
            expectEquals(listed.presets.front().id, juce::String("ambient-sketch"));

            directory.deleteRecursively();
        }

        beginTest("rename updates preset name without changing id");
        {
            const auto directory = uniquePresetDirectory("acestep-presets-rename");
            expect(directory.isDirectory(), "precondition: created preset directory");
            PresetStore store(directory);
            expect(store.save(makePreset()).success);

            const auto renameResult = store.rename("ambient-sketch", "Renamed sketch");
            expect(renameResult.success, renameResult.errorMessage);

            const auto loaded = store.load("ambient-sketch");
            expect(loaded.success, loaded.errorMessage);
            expectEquals(loaded.preset.id, juce::String("ambient-sketch"));
            expectEquals(loaded.preset.name, juce::String("Renamed sketch"));

            directory.deleteRecursively();
        }

        beginTest("delete removes preset file");
        {
            const auto directory = uniquePresetDirectory("acestep-presets-delete");
            expect(directory.isDirectory(), "precondition: created preset directory");
            PresetStore store(directory);
            expect(store.save(makePreset()).success);

            const auto deleteResult = store.deletePreset("ambient-sketch");
            expect(deleteResult.success, deleteResult.errorMessage);
            expect(!store.load("ambient-sketch").success);

            directory.deleteRecursively();
        }

        beginTest("invalid JSON returns explicit failure");
        {
            const auto directory = uniquePresetDirectory("acestep-presets-invalid");
            expect(directory.isDirectory(), "precondition: created preset directory");
            expect(directory.getChildFile("broken.json").replaceWithText("{"));

            PresetStore store(directory);
            const auto invalidResult = store.load("broken");
            expect(!invalidResult.success);
            expect(invalidResult.errorMessage.containsIgnoreCase("invalid"));

            directory.deleteRecursively();
        }

        beginTest("schema version zero migrates to current schema");
        {
            const auto directory = uniquePresetDirectory("acestep-presets-migrate");
            expect(directory.isDirectory(), "precondition: created preset directory");
            expect(directory.getChildFile("legacy.json").replaceWithText(
                R"json({"schemaVersion":0,"id":"legacy","name":"Legacy","prompt":"old prompt"})json"));

            PresetStore store(directory);
            const auto loaded = store.load("legacy");
            expect(loaded.success, loaded.errorMessage);
            expectEquals(loaded.preset.schemaVersion, 1);
            expectEquals(loaded.preset.request.prompt, juce::String("old prompt"));

            directory.deleteRecursively();
        }
    }

private:
    static GenerationPreset makePreset()
    {
        GenerationPreset preset;
        preset.id = "ambient-sketch";
        preset.name = "Ambient sketch";
        preset.request.prompt = "warm ambient pads";
        preset.request.lyrics = "silent stars";
        preset.request.durationSeconds = 42.0f;
        preset.request.seed = 123;
        preset.request.cfgScale = 8.5f;
        preset.request.lmSeed = 456;
        preset.request.scheduler = "dpmpp";
        preset.request.referenceAudioPath = juce::String("C:\\temp\\reference.wav");
        preset.request.stemsEnabled = true;
        preset.request.requestedStemGroups = { StemGroup::vocals, StemGroup::drums };
        preset.request.outputPath = "C:\\temp\\output.wav";
        preset.midiExportRequested = true;
        preset.stemExportRequested = true;
        return preset;
    }

    static juce::File uniquePresetDirectory(const juce::String& prefix)
    {
        const auto file = juce::File::createTempFile(prefix);
        file.deleteFile();
        const auto created = file.createDirectory();
        if (!created)
            return {};
        return file;
    }
};

static PresetStoreTests sPresetStoreTests;

} // namespace acestep_plugin
