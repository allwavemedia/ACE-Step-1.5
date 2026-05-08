#include "SidecarManifest.h"

#include <juce_cryptography/juce_cryptography.h>

namespace acestep_plugin
{

namespace
{

static bool validateArtifact(const juce::var& art)
{
    const juce::String path = art["path"].toString();
    const juce::File artFile(path);

    if (!artFile.existsAsFile())
        return false;

    const auto fileSize = artFile.getSize();
    if (fileSize <= 0)
        return false;

    if (fileSize != static_cast<juce::int64>(art["byteSize"]))
        return false;

    const juce::String recordedHash = art["sha256"].toString();
    if (recordedHash.isEmpty())
        return false;

    const juce::String actualHash = juce::SHA256(artFile).toHexString();
    if (!actualHash.equalsIgnoreCase(recordedHash))
        return false;

    return true;
}

static SidecarProcessError parseAndValidateManifest(const juce::File& manifestFile,
                                                    const juce::String& expectedRequestId,
                                                    juce::StringArray* artifactPaths)
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

        if (artifactPaths != nullptr)
            artifactPaths->add(art["path"].toString());
    }

    return SidecarProcessError::none;
}

} // namespace

SidecarProcessError validateResultManifest(const juce::File& manifestFile,
                                           const juce::String& expectedRequestId)
{
    return parseAndValidateManifest(manifestFile, expectedRequestId, nullptr);
}

juce::StringArray getValidatedArtifactPaths(const juce::File& manifestFile,
                                            const juce::String& expectedRequestId)
{
    juce::StringArray paths;
    if (parseAndValidateManifest(manifestFile, expectedRequestId, &paths)
        != SidecarProcessError::none)
    {
        return {};
    }

    return paths;
}

} // namespace acestep_plugin
