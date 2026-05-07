#include "../Source/Presets/PresetBrowserModel.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class PresetBrowserModelTests final : public juce::UnitTest
{
public:
    PresetBrowserModelTests() : juce::UnitTest("PresetBrowserModel") {}

    void runTest() override
    {
        beginTest("loading a preset updates state without submitting generation");
        {
            const auto directory = uniquePresetDirectory("acestep-preset-browser-model");
            expect(directory.isDirectory(), "precondition: created preset directory");
            PresetStore store(directory);
            expect(store.save(makePreset()).success);

            PresetBrowserModel model(directory);
            int generationSubmitCount = 0;
            model.setGenerationSubmitCallback([&](const GenerationRequest&) {
                ++generationSubmitCount;
            });

            const auto result = model.loadPreset("ambient-sketch");
            expect(result.success, result.errorMessage);
            expect(model.getCurrentPreset().has_value());
            expectEquals(
                model.getCurrentPreset()->request.prompt,
                juce::String("warm ambient pads"));
            expect(model.getCurrentRequest().has_value());
            expectEquals(model.getCurrentRequest()->prompt, juce::String("warm ambient pads"));
            expectEquals(generationSubmitCount, 0);

            directory.deleteRecursively();
        }

        beginTest("refresh keeps valid presets available when another preset is invalid");
        {
            const auto directory = uniquePresetDirectory("acestep-preset-browser-invalid");
            expect(directory.isDirectory(), "precondition: created preset directory");
            PresetStore store(directory);
            expect(store.save(makePreset()).success);
            expect(directory.getChildFile("broken.json").replaceWithText("{"));

            PresetBrowserModel model(directory);
            const auto result = model.refresh();
            expect(result.success, result.errorMessage);
            expect(result.errorMessage.contains("broken"), result.errorMessage);
            expectEquals(static_cast<int>(model.getPresets().size()), 1);
            if (!model.getPresets().empty())
                expectEquals(model.getPresets().front().id, juce::String("ambient-sketch"));

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

static PresetBrowserModelTests sPresetBrowserModelTests;

} // namespace acestep_plugin
