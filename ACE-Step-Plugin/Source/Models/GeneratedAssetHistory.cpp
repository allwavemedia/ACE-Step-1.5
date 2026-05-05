#include "GeneratedAssetHistory.h"

namespace acestep_plugin
{

void GeneratedAssetHistory::add(const GeneratedAsset& asset)
{
    std::lock_guard<std::mutex> lock(mutex);

    // Insert newest at front.
    assets.insert(assets.begin(), asset);

    // Trim to cap.
    if (static_cast<int>(assets.size()) > maxEntries)
        assets.resize(static_cast<std::size_t>(maxEntries));
}

std::vector<GeneratedAsset> GeneratedAssetHistory::getAssets() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return assets;
}

int GeneratedAssetHistory::size() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return static_cast<int>(assets.size());
}

void GeneratedAssetHistory::clear()
{
    std::lock_guard<std::mutex> lock(mutex);
    assets.clear();
}

} // namespace acestep_plugin
