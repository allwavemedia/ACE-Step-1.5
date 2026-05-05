#include "../Source/Engine/AceStepEngine.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class AceStepEngineTests final : public juce::UnitTest
{
public:
    AceStepEngineTests() : juce::UnitTest("AceStepEngine") {}

    void runTest() override
    {
        beginTest("engine is not ready before loadModels");
        {
            AceStepEngine engine;
            expect(!engine.isReady());
        }

        beginTest("loadModels with failing loader leaves engine not ready");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return false; });
            const bool ok = engine.loadModels(juce::File::getSpecialLocation(
                juce::File::tempDirectory));
            expect(!ok);
            expect(!engine.isReady());
        }

        beginTest("loadModels with passing loader marks engine ready");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            const bool ok = engine.loadModels(juce::File::getSpecialLocation(
                juce::File::tempDirectory));
            expect(ok);
            expect(engine.isReady());
        }
    }
};

static AceStepEngineTests sAceStepEngineTests;

} // namespace acestep_plugin
