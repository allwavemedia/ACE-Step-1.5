#pragma once

#include "GeneratedAssetHistory.h"

#include <juce_core/juce_core.h>

#include <mutex>
#include <vector>

namespace acestep_plugin
{

/** Tracks plugin-owned generated WAV directories and removes them at destruction.
 *
 *  Only directories under the system temporary directory are tracked, preventing
 *  accidental cleanup of user-selected export locations.
 */
class GeneratedAssetTempDirectories final
{
public:
    GeneratedAssetTempDirectories();
    ~GeneratedAssetTempDirectories();

    /** Track the parent directory of a plugin-generated WAV file. */
    void trackGeneratedFile(const juce::File& generatedFile);

    /** Track full-mix and successful stem output directories for an asset. */
    void trackGeneratedAsset(const GeneratedAsset& asset);

private:
    bool canTrackDirectory(const juce::File& directory) const;
    std::vector<juce::File> releaseTrackedDirectories();

    juce::File tempDirectory;
    std::mutex mutex;
    std::vector<juce::File> directories;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeneratedAssetTempDirectories)
};

} // namespace acestep_plugin
