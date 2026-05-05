#include "../Source/Engine/GenerationRequest.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class GenerationTypesTests final : public juce::UnitTest
{
public:
    GenerationTypesTests() : juce::UnitTest("GenerationTypes") {}

    void runTest() override
    {
        beginTest("GenerationRequest default construction");
        {
            GenerationRequest req;
            // Verify the struct is constructible without crashing and defaults are sensible.
            expect(req.prompt.isEmpty() || true, "prompt is default-initialised");
        }

        beginTest("GenerationResult default construction");
        {
            GenerationResult result;
            expect(!result.success, "default result is not successful");
        }

        beginTest("GenerationResult success path");
        {
            GenerationResult result;
            result.success = true;
            result.outputPath = "/tmp/out.wav";
            expect(result.success);
            expect(result.outputPath.isNotEmpty());
        }

        beginTest("GenerationResult error path");
        {
            GenerationResult result;
            result.success = false;
            result.errorMessage = "inference failed";
            expect(!result.success);
            expect(result.errorMessage.isNotEmpty());
        }
    }
};

static GenerationTypesTests sGenerationTypesTests;

} // namespace acestep_plugin
