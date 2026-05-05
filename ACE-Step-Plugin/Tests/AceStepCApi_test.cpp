#include "../Source/Engine/AceStepCApi.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class AceStepCApiTests final : public juce::UnitTest
{
public:
    AceStepCApiTests() : juce::UnitTest("AceStepCApi") {}

    void runTest() override
    {
        beginTest("getDefaultBackendPaths returns non-empty directories");
        {
            const auto paths = AceStepCApi::getDefaultBackendPaths();
            // Paths may not exist yet, but should be non-empty strings.
            expect(paths.bundleBinaryDirectory.getFullPathName().isNotEmpty());
            expect(paths.modelsDirectory.getFullPathName().isNotEmpty());
        }

        beginTest("initializeBundledBackends returns false in stub mode");
        {
            // In ACESTEP_PLUGIN_MODE_STUB builds, the backend does nothing.
            const bool result =
                AceStepCApi::initializeBundledBackends(juce::File::getSpecialLocation(
                    juce::File::tempDirectory));
#if ACESTEP_PLUGIN_MODE_STUB
            expect(!result, "stub returns false");
#else
            // In a real build we only assert no exception.
            (void)result;
            expect(true);
#endif
        }
    }
};

static AceStepCApiTests sAceStepCApiTests;

} // namespace acestep_plugin
