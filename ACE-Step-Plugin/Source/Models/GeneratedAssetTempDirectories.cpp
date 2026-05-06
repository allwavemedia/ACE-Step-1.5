#include "GeneratedAssetTempDirectories.h"

#include <algorithm>

namespace acestep_plugin
{

GeneratedAssetTempDirectories::GeneratedAssetTempDirectories()
    : tempDirectory(juce::File::getSpecialLocation(juce::File::tempDirectory))
{
}

GeneratedAssetTempDirectories::~GeneratedAssetTempDirectories()
{
    for (const auto& directory : releaseTrackedDirectories())
        directory.deleteRecursively();
}

void GeneratedAssetTempDirectories::trackGeneratedFile(const juce::File& generatedFile)
{
    const auto directory = generatedFile.getParentDirectory();
    if (!canTrackDirectory(directory))
        return;

    const std::lock_guard<std::mutex> lock(mutex);
    const auto alreadyTracked = std::any_of(
        directories.begin(),
        directories.end(),
        [&directory](const juce::File& existing) {
            return existing.getFullPathName() == directory.getFullPathName();
        });

    if (!alreadyTracked)
        directories.push_back(directory);
}

void GeneratedAssetTempDirectories::trackGeneratedAsset(const GeneratedAsset& asset)
{
    trackGeneratedFile(juce::File(asset.outputPath));

    for (const auto& stem : asset.stems)
        if (stem.success && stem.outputPath.isNotEmpty())
            trackGeneratedFile(juce::File(stem.outputPath));
}

bool GeneratedAssetTempDirectories::canTrackDirectory(const juce::File& directory) const
{
    return directory.exists()
        && directory.isDirectory()
        && directory.isAChildOf(tempDirectory);
}

std::vector<juce::File> GeneratedAssetTempDirectories::releaseTrackedDirectories()
{
    const std::lock_guard<std::mutex> lock(mutex);
    auto released = std::move(directories);
    directories.clear();
    return released;
}

} // namespace acestep_plugin
