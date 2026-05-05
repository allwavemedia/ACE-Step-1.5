#include "../Source/Models/ModelDiscovery.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class ModelDiscoveryTests final : public juce::UnitTest
{
public:
    ModelDiscoveryTests() : juce::UnitTest("ModelDiscovery") {}

    void runTest() override
    {
        beginTest("getRequiredModels returns non-empty list");
        {
            const auto& models = ModelDiscovery::getRequiredModels();
            expect(!models.empty());
        }

        beginTest("getRequiredModels entries have non-empty filenames");
        {
            for (const auto& entry : ModelDiscovery::getRequiredModels())
                expect(entry.filename.isNotEmpty());
        }

        beginTest("getModelsDirectory returns non-empty path");
        {
            const auto dir = ModelDiscovery::getModelsDirectory();
            expect(dir.getFullPathName().isNotEmpty());
        }

        beginTest("getMissingModelFilenames includes all models when dir absent");
        {
            // Unless the user has models installed, all will be reported missing.
            // We just verify the method doesn't crash and returns a list.
            const auto missing = ModelDiscovery::getMissingModelFilenames();
            // Cannot assert the count because a CI machine might have models.
            expect(missing.size() <= ModelDiscovery::getRequiredModels().size());
        }

        beginTest("areAllModelsPresent consistent with getMissingModelFilenames");
        {
            const bool allPresent = ModelDiscovery::areAllModelsPresent();
            const auto missing = ModelDiscovery::getMissingModelFilenames();
            expect(allPresent == missing.empty());
        }
    }
};

static ModelDiscoveryTests sModelDiscoveryTests;

} // namespace acestep_plugin
