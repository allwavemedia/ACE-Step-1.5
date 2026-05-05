#pragma once

#include <juce_core/juce_core.h>

#include <vector>

namespace acestep_plugin
{

/** Describes a single required model file. */
struct ModelEntry
{
    /** Local filename (not path) under the models directory. */
    juce::String filename;

    /** Expected SHA-256 hash as a lowercase hex string. */
    juce::String expectedSha256;

    /** Expected file size in bytes; 0 means unchecked. */
    juce::int64 expectedSize = 0;

    /** Hugging Face download URL. */
    juce::String downloadUrl;
};

/** Discovers and validates ACE-Step model files in the standard local directory.
 *
 *  Model files live under:
 *    Windows: %LOCALAPPDATA%\AceStepPlugin\models\
 *    Other:   ~/Library/Application Support/AceStepPlugin/models/  (macOS)
 *             ~/.local/share/AceStepPlugin/models/                  (Linux)
 */
class ModelDiscovery final
{
public:
    ModelDiscovery() = delete;

    /** Return the manifest of all required GGUF model files. */
    static const std::vector<ModelEntry>& getRequiredModels();

    /** Return the standard local models directory (may not exist). */
    static juce::File getModelsDirectory();

    /** Return true iff all required model files are present (size check only). */
    static bool areAllModelsPresent();

    /** Return filenames of required models that are absent or wrong-sized. */
    static std::vector<juce::String> getMissingModelFilenames();

    /** Approximate total download size in bytes for all missing models. */
    static juce::int64 getMissingTotalBytes();
};

} // namespace acestep_plugin
