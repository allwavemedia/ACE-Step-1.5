#include "../Source/UI/PresetBrowserPanel.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace acestep_plugin
{

class PresetBrowserPanelTests final : public juce::UnitTest
{
public:
    PresetBrowserPanelTests() : juce::UnitTest("PresetBrowserPanel") {}

    void runTest() override
    {
        beginTest("preset list displays available presets");
        {
            PresetBrowserPanel panel;
            GenerationPreset preset;
            preset.id = "ambient-sketch";
            preset.name = "Ambient sketch";

            panel.setPresets({ preset });
            expectEquals(panel.getPresetCount(), 1);
            expectEquals(panel.getSelectedPresetId(), juce::String("ambient-sketch"));
        }

        beginTest("preset list filters by preset name");
        {
            PresetBrowserPanel panel;
            GenerationPreset ambient;
            ambient.id = "ambient-sketch";
            ambient.name = "Ambient sketch";
            GenerationPreset drums;
            drums.id = "drum-sketch";
            drums.name = "Drum sketch";

            panel.setPresets({ ambient, drums });
            panel.setFilterText("drum");
            expectEquals(panel.getPresetCount(), 1);
            expectEquals(panel.getSelectedPresetId(), juce::String("drum-sketch"));
        }
    }
};

static PresetBrowserPanelTests sPresetBrowserPanelTests;

} // namespace acestep_plugin
