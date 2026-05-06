/// @file GeneratedAssetTempDirectoryCleanup_test.cpp
/// Tests plugin-owned temporary generation directory cleanup on destruction.

#include "../Source/Models/GeneratedAssetTempDirectories.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class GeneratedAssetTempDirectoryCleanupTests final : public juce::UnitTest
{
public:
    GeneratedAssetTempDirectoryCleanupTests()
        : juce::UnitTest("GeneratedAssetTempDirectoryCleanup")
    {
    }

    void runTest() override
    {
        beginTest("plugin-owned directory is deleted on destruction");
        {
            const auto tempRoot = uniqueTempRoot("acestep-test-cleanup");
            const auto ownedDirectory = tempRoot.getChildFile("plugin-owned-generation");
            const auto generatedWav = ownedDirectory.getChildFile("generated.wav");

            expect(ownedDirectory.createDirectory(), "precondition: created owned directory");
            expect(generatedWav.create(), "precondition: created generated WAV");

            {
                GeneratedAssetTempDirectories directories;
                directories.trackGeneratedFile(generatedWav);
            }

            expect(!ownedDirectory.exists(), "plugin-owned directory removed after destruction");
            tempRoot.deleteRecursively();
        }

        beginTest("user-saved copy outside plugin directory is preserved");
        {
            const auto tempRoot = uniqueTempRoot("acestep-test-preserve");
            const auto ownedDirectory = tempRoot.getChildFile("plugin-owned-generation");
            const auto generatedWav = ownedDirectory.getChildFile("generated.wav");
            const auto savedCopy = tempRoot.getChildFile("user-saved-copy").getChildFile("my-song.wav");

            expect(ownedDirectory.createDirectory(), "precondition: created owned directory");
            expect(generatedWav.create(), "precondition: created generated WAV");
            expect(savedCopy.getParentDirectory().createDirectory(),
                "precondition: created save directory");
            expect(savedCopy.create(), "precondition: created user-saved copy");

            {
                GeneratedAssetTempDirectories directories;
                directories.trackGeneratedFile(generatedWav);
            }

            expect(!ownedDirectory.exists(), "plugin-owned directory removed after destruction");
            expect(savedCopy.exists(), "user-saved copy outside owned directory is untouched");
            tempRoot.deleteRecursively();
        }
    }

private:
    static juce::File uniqueTempRoot(const juce::String& prefix)
    {
        const auto root = juce::File::getSpecialLocation(juce::File::tempDirectory)
                              .getChildFile(prefix + "-" + juce::Uuid().toString());
        root.deleteRecursively();
        return root;
    }
};

static GeneratedAssetTempDirectoryCleanupTests sGeneratedAssetTempDirectoryCleanupTests;

} // namespace acestep_plugin
