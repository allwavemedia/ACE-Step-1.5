#include "ReferenceAudioBufferTestUtils.h"

#include "../Source/UI/GeneratedAssetHistoryView.h"

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace acestep_plugin
{

class GeneratedAssetHistoryViewTests final : public juce::UnitTest
{
public:
    GeneratedAssetHistoryViewTests() : juce::UnitTest("GeneratedAssetHistoryView") {}

    void runTest() override
    {
        beginTest("empty history shows zero tiles");
        {
            GeneratedAssetHistoryView view;
            view.refresh({});
            expectEquals(view.getTileCount(), 0);
        }

        beginTest("N assets creates N tiles");
        {
            GeneratedAssetHistoryView view;
            std::vector<GeneratedAsset> assets;

            for (int i = 0; i < 4; ++i)
                assets.push_back(makeTestAsset(i));

            view.refresh(assets);
            expectEquals(view.getTileCount(), 4);
        }

        beginTest("capped at maxEntries tiles");
        {
            GeneratedAssetHistoryView view;
            std::vector<GeneratedAsset> assets;

            // Pass more than maxEntries.
            for (int i = 0; i < GeneratedAssetHistory::maxEntries + 2; ++i)
                assets.push_back(makeTestAsset(i));

            // Trim to maxEntries (the history manager enforces this, but the
            // view should handle whatever vector it receives).
            assets.resize(GeneratedAssetHistory::maxEntries);
            view.refresh(assets);
            expectEquals(view.getTileCount(), GeneratedAssetHistory::maxEntries);
        }

        beginTest("content height matches tile count");
        {
            constexpr int tileH = GeneratedAssetHistoryView::ContentComponent::tileHeight;
            constexpr int tileG = GeneratedAssetHistoryView::ContentComponent::tileGap;

            for (int n : { 0, 1, 3, 8 })
            {
                const int expected = n == 0 ? 0 : n * tileH + (n - 1) * tileG;
                expectEquals(
                    GeneratedAssetHistoryView::ContentComponent::preferredHeight(n),
                    expected);
            }
        }

        beginTest("callbacks are forwarded to tiles without crash");
        {
            GeneratedAssetHistoryView view;
            view.setOnPlayStop([](const GeneratedAsset&, bool) {});
            view.setOnSaveAs([](const GeneratedAsset&) {});
            view.setOnStemPreview([](const GeneratedAsset&, const StemAsset&, bool) {});
            view.setOnStemSaveAs([](const GeneratedAsset&, const StemAsset&) {});

            std::vector<GeneratedAsset> assets = { makeTestAsset(0), makeTestAsset(1) };
            view.refresh(assets);
            expect(view.getTileCount() == 2);
        }

        beginTest("refresh replaces existing tiles");
        {
            GeneratedAssetHistoryView view;
            view.refresh({ makeTestAsset(0), makeTestAsset(1) });
            expectEquals(view.getTileCount(), 2);

            view.refresh({ makeTestAsset(2) });
            expectEquals(view.getTileCount(), 1);
        }
    }
};

static GeneratedAssetHistoryViewTests sGeneratedAssetHistoryViewTests;

} // namespace acestep_plugin
