#pragma once

#include "GeneratedAssetTile.h"
#include "../Models/GeneratedAssetHistory.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include <vector>

namespace acestep_plugin
{

/** Scrollable panel showing the generated-asset history.
 *
 *  Wraps a juce::Viewport around a ContentComponent that lays up to
 *  GeneratedAssetHistory::maxEntries GeneratedAssetTile instances
 *  vertically.  Call refresh() whenever the history changes to rebuild
 *  the tile list and resize the content.
 */
class GeneratedAssetHistoryView final : public juce::Component
{
public:
    GeneratedAssetHistoryView();
    ~GeneratedAssetHistoryView() override = default;

    /** Rebuild tiles from the provided asset list (newest first). */
    void refresh(const std::vector<GeneratedAsset>& assets);

    /** Return the number of tiles currently displayed. */
    int getTileCount() const;

    void setOnPlayStop(GeneratedAssetTile::PlayStopCallback callback);
    void setOnSaveAs(GeneratedAssetTile::SaveAsCallback callback);

    void resized() override;
    void paint(juce::Graphics& g) override;

    /** Inner component scrolled by the Viewport.
     *
     *  Declared public so tests can access tile-layout constants and
     *  preferredHeight() without requiring a friend declaration.
     */
    class ContentComponent final : public juce::Component
    {
    public:
        static constexpr int tileHeight = 110;
        static constexpr int tileGap = 4;

        void refresh(
            const std::vector<GeneratedAsset>& assets,
            GeneratedAssetTile::PlayStopCallback onPlayStop,
            GeneratedAssetTile::SaveAsCallback onSaveAs);

        int getTileCount() const noexcept { return static_cast<int>(tiles.size()); }

        /** Height needed for n tiles including gaps. */
        static int preferredHeight(int numTiles) noexcept;

        void resized() override;

    private:
        std::vector<std::unique_ptr<GeneratedAssetTile>> tiles;
    };

private:
    juce::Viewport viewport;
    ContentComponent content;

    GeneratedAssetTile::PlayStopCallback onPlayStopCallback;
    GeneratedAssetTile::SaveAsCallback onSaveAsCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratedAssetHistoryView)
};

} // namespace acestep_plugin
