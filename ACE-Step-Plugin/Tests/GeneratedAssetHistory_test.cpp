#include "ReferenceAudioBufferTestUtils.h"

#include "../Source/Models/GeneratedAssetHistory.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

class GeneratedAssetHistoryTests final : public juce::UnitTest
{
public:
    GeneratedAssetHistoryTests() : juce::UnitTest("GeneratedAssetHistory") {}

    void runTest() override
    {
        beginTest("empty history has size zero");
        {
            GeneratedAssetHistory history;
            expectEquals(history.size(), 0);
        }

        beginTest("add increases size");
        {
            GeneratedAssetHistory history;
            history.add(makeTestAsset(0));
            expectEquals(history.size(), 1);
        }

        beginTest("newest asset is first");
        {
            GeneratedAssetHistory history;
            history.add(makeTestAsset(0));
            history.add(makeTestAsset(1));
            const auto assets = history.getAssets();
            expectEquals(assets[0].id, juce::String("test-asset-1"));
        }

        beginTest("capped at maxEntries");
        {
            GeneratedAssetHistory history;
            for (int i = 0; i < GeneratedAssetHistory::maxEntries + 3; ++i)
                history.add(makeTestAsset(i));

            expectEquals(history.size(), GeneratedAssetHistory::maxEntries);
        }

        beginTest("clear empties history");
        {
            GeneratedAssetHistory history;
            history.add(makeTestAsset(0));
            history.clear();
            expectEquals(history.size(), 0);
        }

        beginTest("history preserves grouped stem assets");
        {
            GeneratedAssetHistory history;
            auto asset = makeTestAsset(0);
            asset.stems.push_back(StemAsset { StemGroup::vocals, "C:\\temp\\vocals.wav", true, {} });

            history.add(asset);

            const auto assets = history.getAssets();
            expectEquals(static_cast<int>(assets.front().stems.size()), 1);
            expectEquals(assets.front().stems.front().outputPath, juce::String("C:\\temp\\vocals.wav"));
        }
    }
};

static GeneratedAssetHistoryTests sGeneratedAssetHistoryTests;

} // namespace acestep_plugin
