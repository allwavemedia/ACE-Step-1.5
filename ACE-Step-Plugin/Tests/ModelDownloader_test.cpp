#include "../Source/Models/ModelDownloader.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ModelDownloaderTests final : public juce::UnitTest
{
public:
    ModelDownloaderTests() : juce::UnitTest("ModelDownloader") {}

    void runTest() override
    {
        beginTest("isRunning is false after construction");
        {
            ModelDownloader dl;
            expect(!dl.isRunning());
        }

        beginTest("start with empty model list calls completion immediately");
        {
            ModelDownloader dl;
            std::atomic<bool> completionFired { false };
            std::atomic<bool> completionSuccess { false };

            dl.start(
                {},
                juce::File::getSpecialLocation(juce::File::tempDirectory),
                nullptr,
                [&](bool success, const juce::String&) {
                    completionSuccess.store(success);
                    completionFired.store(true);
                });

            // Wait up to 2 seconds for the worker to call back.
            for (int i = 0; i < 200 && !completionFired.load(); ++i)
                juce::Thread::sleep(10);

            expect(completionFired.load(), "completion callback fired");
            expect(completionSuccess.load(), "empty list succeeds");
        }

        beginTest("cancel stops isRunning");
        {
            ModelDownloader dl;
            dl.cancel(); // No-op when not running; should not crash.
            expect(!dl.isRunning());
        }
    }
};

static ModelDownloaderTests sModelDownloaderTests;

} // namespace acestep_plugin
