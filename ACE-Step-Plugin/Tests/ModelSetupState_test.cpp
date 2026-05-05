#include "../Source/Models/ModelSetupState.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ModelSetupStateTests final : public juce::UnitTest
{
public:
    ModelSetupStateTests() : juce::UnitTest("ModelSetupState") {}

    void runTest() override
    {
        beginTest("initial phase is Idle");
        {
            ModelSetupStateManager mgr;
            expectEquals(
                static_cast<int>(mgr.getState().phase),
                static_cast<int>(ModelSetupPhase::Idle));
        }

        beginTest("isReady returns false for Idle phase");
        {
            ModelSetupStateManager mgr;
            expect(!mgr.isReady());
        }

        beginTest("setState to Ready marks isReady true");
        {
            ModelSetupStateManager mgr;
            ModelSetupState s;
            s.phase = ModelSetupPhase::Ready;
            mgr.setState(s);
            expect(mgr.isReady());
        }

        beginTest("setState to Failed marks isReady false");
        {
            ModelSetupStateManager mgr;
            ModelSetupState s;
            s.phase = ModelSetupPhase::Failed;
            s.errorMessage = "disk full";
            mgr.setState(s);
            expect(!mgr.isReady());
            expectEquals(mgr.getState().errorMessage, juce::String("disk full"));
        }

        beginTest("downloading phase message propagates");
        {
            ModelSetupStateManager mgr;
            ModelSetupState s;
            s.phase = ModelSetupPhase::Downloading;
            s.currentFilename = "ace-step-v1.Q8_0.gguf";
            s.overallProgress = 0.5f;
            mgr.setState(s);
            expect(!mgr.isReady());
            expectEquals(mgr.getState().currentFilename,
                         juce::String("ace-step-v1.Q8_0.gguf"));
        }
    }
};

static ModelSetupStateTests sModelSetupStateTests;

} // namespace acestep_plugin
