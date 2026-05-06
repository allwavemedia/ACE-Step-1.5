#include "../Source/Engine/AceStepEngine.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class AceStepEngineProgressTests final : public juce::UnitTest
{
public:
    AceStepEngineProgressTests() : juce::UnitTest("AceStepEngineProgress") {}

    void runTest() override
    {
        beginTest("progress callback receives messages from runner");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            engine.loadModels({});

            juce::StringArray received;
            juce::CriticalSection cs;

            engine.setProgressCallback([&](const juce::String& msg) {
                juce::ScopedLock sl(cs);
                received.add(msg);
            });

            engine.setGenerationRunner(
                [&engine](const GenerationRequest&, const std::atomic<bool>&) {
                    engine.reportProgress("step 1");
                    engine.reportProgress("step 2");
                    return GenerationResult {};
                });

            engine.submitAsync({}, [](const GenerationResult&) {});
            engine.waitForAllJobs();

            juce::ScopedLock sl(cs);
            expect(received.contains("step 1"));
            expect(received.contains("step 2"));
        }

        beginTest("no progress callback does not crash");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            engine.loadModels({});

            engine.setGenerationRunner(
                [&engine](const GenerationRequest&, const std::atomic<bool>&) {
                    engine.reportProgress("ignored");
                    return GenerationResult {};
                });

            engine.submitAsync({}, [](const GenerationResult&) {});
            engine.waitForAllJobs();
            expect(true, "no crash without progress callback");
        }

        beginTest("stem progress is reported separately from full mix progress");
        {
            AceStepEngine engine;
            engine.setBackendLoader([](const juce::File&) { return true; });
            engine.loadModels({});

            juce::StringArray received;
            juce::CriticalSection cs;

            engine.setProgressCallback([&](const juce::String& msg) {
                juce::ScopedLock sl(cs);
                received.add(msg);
            });

            engine.setGenerationRunner(
                [&engine](const GenerationRequest&, const std::atomic<bool>&) {
                    engine.reportProgress("Full mix: complete");
                    engine.reportStemProgress(StemGroup::vocals, "separating");
                    return GenerationResult {};
                });

            engine.submitAsync({}, [](const GenerationResult&) {});
            engine.waitForAllJobs();

            juce::ScopedLock sl(cs);
            expect(received.contains("Full mix: complete"));
            expect(received.contains("Stem vocals: separating"));
        }
    }
};

static AceStepEngineProgressTests sAceStepEngineProgressTests;

} // namespace acestep_plugin
