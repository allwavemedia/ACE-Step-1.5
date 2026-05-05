#include "../Source/Engine/AceStepEngine.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

/** Tests AceStepEngine with fully mock runner — no real inference. */
class AceStepEngineMockTests final : public juce::UnitTest
{
public:
    AceStepEngineMockTests() : juce::UnitTest("AceStepEngineMock") {}

    void runTest() override
    {
        beginTest("mock runner returns injected result");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            engine.loadModels({});

            GenerationResult expected;
            expected.success = true;
            expected.outputPath = "/tmp/mock_output.wav";

            engine.setGenerationRunner(
                [expected](const GenerationRequest&, const std::atomic<bool>&) {
                    return expected;
                });

            const auto actual = engine.generate({});
            expect(actual.success);
            expectEquals(actual.outputPath, expected.outputPath);
        }

        beginTest("generate returns error result when runner throws nothing");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            engine.loadModels({});

            engine.setGenerationRunner(
                [](const GenerationRequest&, const std::atomic<bool>&) {
                    GenerationResult r;
                    r.success = false;
                    r.errorMessage = "mock error";
                    return r;
                });

            const auto result = engine.generate({});
            expect(!result.success);
            expect(result.errorMessage.isNotEmpty());
        }

        beginTest("cancelAll sets cancel flag seen by runner");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            engine.loadModels({});

            std::atomic<bool> sawCancel { false };
            engine.setGenerationRunner(
                [&sawCancel](const GenerationRequest&, const std::atomic<bool>& cancel) {
                    for (int i = 0; i < 100; ++i)
                    {
                        if (cancel.load())
                        {
                            sawCancel.store(true);
                            break;
                        }
                        juce::Thread::sleep(1);
                    }
                    return GenerationResult {};
                });

            engine.submitAsync({}, [](const GenerationResult&) {});
            juce::Thread::sleep(10);
            engine.cancelAll();
            engine.waitForAllJobs();
            expect(sawCancel.load());
        }
    }
};

static AceStepEngineMockTests sAceStepEngineMockTests;

} // namespace acestep_plugin
