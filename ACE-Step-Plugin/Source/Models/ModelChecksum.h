#pragma once

#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>

namespace acestep_plugin
{

/** SHA-256 utilities for model file integrity verification. */
class ModelChecksum final
{
public:
    ModelChecksum() = delete;

    /** Compute the SHA-256 hash of a file and return it as a lowercase hex string.
     *
     *  Returns an empty string if the file cannot be read.
     */
    static juce::String computeForFile(const juce::File& file);

    /** Return true iff the file's SHA-256 matches expectedHex (case-insensitive).
     *
     *  Returns false for unreadable or missing files.
     */
    static bool verifyFile(const juce::File& file, const juce::String& expectedHex);
};

} // namespace acestep_plugin
