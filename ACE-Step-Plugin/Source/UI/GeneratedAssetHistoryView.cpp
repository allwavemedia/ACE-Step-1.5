#include "GeneratedAssetHistoryView.h"

namespace acestep_plugin
{

// ---------------------------------------------------------------------------
// ContentComponent
// ---------------------------------------------------------------------------

int GeneratedAssetHistoryView::ContentComponent::preferredHeight(int numTiles) noexcept
{
    if (numTiles <= 0)
        return 0;

    return numTiles * tileHeight + (numTiles - 1) * tileGap;
}

void GeneratedAssetHistoryView::ContentComponent::refresh(
    const std::vector<GeneratedAsset>& assets,
    GeneratedAssetTile::PlayStopCallback onPlayStop,
    GeneratedAssetTile::SaveAsCallback onSaveAs)
{
    tiles.clear();

    for (const auto& asset : assets)
    {
        auto tile = std::make_unique<GeneratedAssetTile>(asset);

        if (onPlayStop)
            tile->setOnPlayStop(onPlayStop);

        if (onSaveAs)
            tile->setOnSaveAs(onSaveAs);

        addAndMakeVisible(*tile);
        tiles.push_back(std::move(tile));
    }

    const int totalHeight = preferredHeight(static_cast<int>(tiles.size()));
    setSize(getWidth() > 0 ? getWidth() : 1, totalHeight > 0 ? totalHeight : 1);

    resized();
}

void GeneratedAssetHistoryView::ContentComponent::resized()
{
    int y = 0;

    for (auto& tile : tiles)
    {
        tile->setBounds(0, y, getWidth(), tileHeight);
        y += tileHeight + tileGap;
    }
}

// ---------------------------------------------------------------------------
// GeneratedAssetHistoryView
// ---------------------------------------------------------------------------

GeneratedAssetHistoryView::GeneratedAssetHistoryView()
{
    viewport.setViewedComponent(&content, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);
}

void GeneratedAssetHistoryView::refresh(const std::vector<GeneratedAsset>& assets)
{
    content.refresh(assets, onPlayStopCallback, onSaveAsCallback);

    const int contentWidth = viewport.getMaximumVisibleWidth();
    content.setSize(contentWidth > 0 ? contentWidth : 1,
                    ContentComponent::preferredHeight(static_cast<int>(assets.size())));

    repaint();
}

int GeneratedAssetHistoryView::getTileCount() const
{
    return content.getTileCount();
}

void GeneratedAssetHistoryView::setOnPlayStop(GeneratedAssetTile::PlayStopCallback callback)
{
    onPlayStopCallback = std::move(callback);
}

void GeneratedAssetHistoryView::setOnSaveAs(GeneratedAssetTile::SaveAsCallback callback)
{
    onSaveAsCallback = std::move(callback);
}

void GeneratedAssetHistoryView::resized()
{
    viewport.setBounds(getLocalBounds());

    // Keep content width in sync with the viewport's visible width.
    const int w = viewport.getMaximumVisibleWidth();

    if (w > 0)
        content.setSize(w, content.getHeight());
}

void GeneratedAssetHistoryView::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

} // namespace acestep_plugin
