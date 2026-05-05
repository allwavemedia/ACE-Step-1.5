#include "ModelDiscovery.h"

namespace acestep_plugin
{

namespace
{

// Required GGUF model manifest for the "v1-turbo-q5" profile.
// Source: Resources/model_manifest.json (Serveurperso/ACE-Step-1.5-GGUF).
const std::vector<ModelEntry> kRequiredModels = {
    {
        "acestep-5Hz-lm-4B-Q5_K_M.gguf",
        "938ed7067c8897f66acf4c3a86fc1fa8113d5cd1a5f13e6edec2e03207514e2d",
        3025965984LL,
        "https://huggingface.co/Serveurperso/ACE-Step-1.5-GGUF/resolve/main/"
        "acestep-5Hz-lm-4B-Q5_K_M.gguf",
    },
    {
        "acestep-v15-turbo-Q5_K_M.gguf",
        "a241c9a721e3704cb04b17ce6a40c9aa714d3ee5cf49c2219972020eb761f5a4",
        1700140224LL,
        "https://huggingface.co/Serveurperso/ACE-Step-1.5-GGUF/resolve/main/"
        "acestep-v15-turbo-Q5_K_M.gguf",
    },
    {
        "Qwen3-Embedding-0.6B-Q8_0.gguf",
        "972f23255e46adfe744a0eb9a0039f3c63988f65753b0968d776e8b27168c321",
        784144960LL,
        "https://huggingface.co/Serveurperso/ACE-Step-1.5-GGUF/resolve/main/"
        "Qwen3-Embedding-0.6B-Q8_0.gguf",
    },
    {
        "vae-BF16.gguf",
        "0599862ac5d15cd308e1d2e368373aea6c02e25ebd1737ad4a4562a0901b0ef8",
        337420928LL,
        "https://huggingface.co/Serveurperso/ACE-Step-1.5-GGUF/resolve/main/"
        "vae-BF16.gguf",
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
