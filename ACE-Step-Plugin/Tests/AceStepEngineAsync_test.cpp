#include "../Source/Engine/AceStepEngine.h"

#include <juce_core/juce_core.h>

#include <atomic>

namespace acestep_plugin
{

class AceStepEngineAsyncTests final : public juce::UnitTest
{
public:
    AceStepEngineAsyncTests() : juce::UnitTest("AceStepEngineAsync") {}

    void runTest() override
    {
        beginTest("submitAsync returns false when engine not ready");
        {
            AceStepEngine engine;
            GenerationRequest req;
            bool callbackFired = false;
            const bool submitted = engine.submitAsync(req, [&](const GenerationResult&) {
                callbackFired = true;
            });
            expect(!submitted);
            engine.waitForAllJobs();
            expect(!callbackFired);
        }

        beginTest("submitAsync invokes callback with ready engine");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            engine.loadModels({});

            GenerationResult fakeResult;
            fakeResult.success = true;
            engine.setGenerationRunner(
                [fakeResult](const GenerationRequest&, const std::atomic<bool>&) {
                    return fakeResult;
                });

            std::atomic<bool> callbackFired { false };
            GenerationResult receivedResult;

            const bool submitted = engine.submitAsync(
                {},
                [&](const GenerationResult& r) {
                    receivedResult = r;
                    callbackFired.store(true);
                });

            expect(submitted);
            engine.waitForAllJobs();
            expect(callbackFired.load());
            expect(receivedResult.success);
        }

        beginTest("second submitAsync while job running returns false");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            engine.loadModels({});

            engine.setGenerationRunner([](const GenerationRequest&, const std::atomic<bool>&) {
                juce::Thread::sleep(50);
                return GenerationResult {};
            });

            const bool first = engine.submitAsync({}, [](const GenerationResult&) {});
            const bool second = engine.submitAsync({}, [](const GenerationResult&) {});

            expect(first);
            expect(!second);
            engine.waitForAllJobs();
        }
    }
};

static AceStepEngineAsyncTests sAceStepEngineAsyncTests;

} // namespace acestep_plugin
