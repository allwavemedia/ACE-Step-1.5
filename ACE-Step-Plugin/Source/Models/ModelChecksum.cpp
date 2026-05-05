#include "ModelChecksum.h"

namespace acestep_plugin
{

juce::String ModelChecksum::computeForFile(const juce::File& file)
{
    auto stream = file.createInputStream();

    if (stream == nullptr)
        return {};

    juce::SHA256 sha256(*stream);
    return sha256.toHexString();
}

bool ModelChecksum::verifyFile(const juce::File& file, const juce::String& expectedHex)
{
    if (expectedHex.isEmpty())
        return false;

    const auto actual = computeForFile(file);

    if (actual.isEmpty())
        return false;

    return actual.equalsIgnoreCase(expectedHex);
}

} // namespace acestep_plugin
