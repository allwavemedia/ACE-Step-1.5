#include "../Source/Models/ModelDownloader.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ModelDownloaderErrorTests final : public juce::UnitTest
{
public:
    ModelDownloaderErrorTests() : juce::UnitTest("ModelDownloaderErrors") {}

    void runTest() override
    {
        beginTest("download of unreachable URL calls completion with failure");
        {
            ModelEntry badEntry;
            badEntry.filename = "nonexistent.gguf";
            badEntry.downloadUrl = "http://localhost:1/nonexistent.gguf";
            badEntry.expectedSize = 1024;

            ModelDownloader dl;
            std::atomic<bool> completionFired { false };
            std::atomic<bool> completionSuccess { true };
            juce::String errorMsg;

            dl.start(
                { badEntry },
                juce::File::getSpecialLocation(juce::File::tempDirectory),
                nullptr,
                [&](bool success, const juce::String& err) {
                    completionSuccess.store(success);
                    errorMsg = err;
                    completionFired.store(true);
                });

            for (int i = 0; i < 500 && !completionFired.load(); ++i)
                juce::Thread::sleep(10);

            expect(completionFired.load(), "completion callback fired");
            expect(!completionSuccess.load(), "bad URL causes failure");
            expect(errorMsg.isNotEmpty(), "error message provided");
        }

        beginTest("cancel during download sets failure path");
        {
            ModelEntry badEntry;
            badEntry.filename = "nonexistent.gguf";
            badEntry.downloadUrl = "http://localhost:1/nonexistent.gguf";

            ModelDownloader dl;
            std::atomic<bool> completionFired { false };

            dl.start(
                { badEntry },
                juce::File::getSpecialLocation(juce::File::tempDirectory),
                nullptr,
                [&](bool, const juce::String&) { completionFired.store(true); });

            dl.cancel();

            for (int i = 0; i < 300 && !completionFired.load(); ++i)
                juce::Thread::sleep(10);

            expect(completionFired.load(), "completion fires after cancel");
        }
    }
};

static ModelDownloaderErrorTests sModelDownloaderErrorTests;

} // namespace acestep_plugin
