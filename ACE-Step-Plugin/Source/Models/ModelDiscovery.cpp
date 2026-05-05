#include "ModelDiscovery.h"

namespace acestep_plugin
{

namespace
{

// Required GGUF model manifest.
// SHA-256 and URL values are placeholders; update with real values at release.
const std::vector<ModelEntry> kRequiredModels = {
    {
        "ace-step-v1-vocal-expert.Q8_0.gguf",
        "", // SHA-256 TBD
        0,  // size TBD
        "https://huggingface.co/allwavemedia/ACE-Step-GGUF/resolve/main/"
        "ace-step-v1-vocal-expert.Q8_0.gguf",
    },
    {
        "ace-step-v1.Q8_0.gguf",
        "",
        0,
        "https://huggingface.co/allwavemedia/ACE-Step-GGUF/resolve/main/"
        "ace-step-v1.Q8_0.gguf",
    },
};

} // namespace

const std::vector<ModelEntry>& ModelDiscovery::getRequiredModels()
{
    return kRequiredModels;
}

juce::File ModelDiscovery::getModelsDirectory()
{
    const auto localAppData =
        juce::SystemStats::getEnvironmentVariable("LOCALAPPDATA", {});

    if (localAppData.isNotEmpty())
        return juce::File(localAppData).getChildFile("AceStepPlugin").getChildFile("models");

    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("AceStepPlugin")
        .getChildFile("models");
}

bool ModelDiscovery::areAllModelsPresent()
{
    return getMissingModelFilenames().empty();
}

std::vector<juce::String> ModelDiscovery::getMissingModelFilenames()
{
    const auto dir = getModelsDirectory();
    std::vector<juce::String> missing;

    for (const auto& entry : getRequiredModels())
    {
        const auto file = dir.getChildFile(entry.filename);

        if (!file.existsAsFile())
        {
            missing.push_back(entry.filename);
            continue;
        }

        if (entry.expectedSize > 0 && file.getSize() != entry.expectedSize)
            missing.push_back(entry.filename);
    }

    return missing;
}

juce::int64 ModelDiscovery::getMissingTotalBytes()
{
    const auto dir = getModelsDirectory();
    juce::int64 total = 0;

    for (const auto& entry : getRequiredModels())
    {
        const auto file = dir.getChildFile(entry.filename);

        if (!file.existsAsFile() || (entry.expectedSize > 0 && file.getSize() != entry.expectedSize))
            total += entry.expectedSize;
    }

    return total;
}

} // namespace acestep_plugin
