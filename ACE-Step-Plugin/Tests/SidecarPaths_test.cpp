#include "../Source/Sidecar/SidecarPaths.h"

#include <juce_core/juce_core.h>

namespace
{
class SidecarPathsTests final : public juce::UnitTest
{
public:
    SidecarPathsTests() : juce::UnitTest("SidecarPaths") {}

    void runTest() override
    {
        beginTest("creates plugin-owned job root under local app data");
        {
            const auto root = acestep_plugin::SidecarPaths::getJobRoot();
            expect(root.getFullPathName().contains("AceStepPlugin"));
            expect(root.getFileName() == "jobs");
        }

        beginTest("builds unique job directory names");
        {
            const auto first = acestep_plugin::SidecarPaths::createJobDirectory("generation");
            const auto second = acestep_plugin::SidecarPaths::createJobDirectory("generation");
            expect(first.exists());
            expect(second.exists());
            expect(first != second);
            first.deleteRecursively();
            second.deleteRecursively();
        }
    }
};

static SidecarPathsTests sSidecarPathsTests;
} // namespace
