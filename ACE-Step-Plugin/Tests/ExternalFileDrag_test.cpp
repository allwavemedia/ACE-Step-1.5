#include "../Source/UI/ExternalFileDrag.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ExternalFileDragTests final : public juce::UnitTest
{
public:
    ExternalFileDragTests() : juce::UnitTest("ExternalFileDrag") {}

    void runTest() override
    {
        beginTest("startCopyDrag with injected performer uses copy semantics");
        {
            bool performerCalled = false;
            bool receivedCopyFlag = true;

            ExternalFileDrag::Performer performer =
                [&](const juce::StringArray& files, bool canMoveFiles) -> bool {
                performerCalled = true;
                receivedCopyFlag = canMoveFiles;
                expect(!files.isEmpty());
                return true;
            };

            // Use a temp file so the existence check passes.
            juce::TemporaryFile tmp(".wav");
            tmp.getFile().create();

            const bool result =
                ExternalFileDrag::startCopyDrag(tmp.getFile(), performer);

            expect(performerCalled);
            expect(!receivedCopyFlag, "canMoveFiles should be false for copy drag");
            expect(result);
        }

        beginTest("startCopyDrag rejects missing files");
        {
            ExternalFileDrag::Performer performer =
                [](const juce::StringArray&, bool) -> bool { return true; };

            const bool result =
                ExternalFileDrag::startCopyDrag(
                    juce::File("/nonexistent/path/missing.wav"), performer);

            expect(!result);
        }
    }
};

static ExternalFileDragTests sExternalFileDragTests;

} // namespace acestep_plugin
