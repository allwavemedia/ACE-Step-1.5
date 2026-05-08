#include "SidecarManifest.h"

#include <juce_cryptography/juce_cryptography.h>

namespace acestep_plugin
{

static bool validateArtifact(const juce::var& art)
{
    const juce::String path = art["path"].toString();
    const juce::File artFile(path);

    if (!artFile.existsAsFile())
        return false;

    if (artFile.getSize() != static_cast<juce::int64>(art["byteSize"]))
        return false;

    const juce::String recordedHash = art["sha256"].toString();
    if (recordedHash.isNotEmpty())
    {
        const juce::String actualHash = juce::SHA256(artFile).toHexString();
        if (!actualHash.equalsIgnoreCase(recordedHash))
            return false;
    }

    return true;
}

SidecarProcessError validateResultManifest(const juce::File& manifestFile,
                                           const juce::String& expectedRequestId)
{
    if (!manifestFile.existsAsFile())
        return SidecarProcessError::manifestInvalid;

    juce::var parsed;
    if (juce::JSON::parse(manifestFile.loadFileAsString(), parsed).failed())
        return SidecarProcessError::manifestInvalid;

    const auto* obj = parsed.getDynamicObject();
    if (obj == nullptr)
        return SidecarProcessError::manifestInvalid;

    if (obj->getProperty("protocolVersion").toString().isEmpty())
        return SidecarProcessError::manifestInvalid;

    if (obj->getProperty("requestId").toString() != expectedRequestId)
        return SidecarProcessError::manifestInvalid;

    if (!static_cast<bool>(obj->getProperty("success")))
        return SidecarProcessError::manifestInvalid;

    const juce::var artifactsVar = obj->getProperty("artifacts");
    if (!artifactsVar.isArray())
        return SidecarProcessError::manifestInvalid;

    const auto* artifacts = artifactsVar.getArray();
    if (artifacts->isEmpty())
        return SidecarProcessError::manifestInvalid;

    for (const auto& art : *artifacts)
    {
        if (!validateArtifact(art))
            return SidecarProcessError::manifestInvalid;
    }

    return SidecarProcessError::none;
}

juce::StringArray getValidatedArtifactPaths(const juce::File& manifestFile,
                                            const juce::String& expectedRequestId)
{
    if (validateResultManifest(manifestFile, expectedRequestId) != SidecarProcessError::none)
        return {};

    juce::var parsed;
    if (juce::JSON::parse(manifestFile.loadFileAsString(), parsed).failed())
        return {};

    const auto* obj = parsed.getDynamicObject();
    if (obj == nullptr)
        return {};

    const juce::var artifactsVar = obj->getProperty("artifacts");
    const auto* artifacts = artifactsVar.getArray();
    if (artifacts == nullptr || artifacts->isEmpty())
        return {};

    juce::StringArray paths;
    for (const auto& art : *artifacts)
        paths.add(art["path"].toString());

    return paths;
}

} // namespace acestep_plugin
