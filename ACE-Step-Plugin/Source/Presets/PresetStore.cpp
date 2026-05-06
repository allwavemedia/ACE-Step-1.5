#include "PresetStore.h"

namespace acestep_plugin
{
namespace
{

constexpr int kCurrentSchemaVersion = 1;

juce::Identifier idFor(const char* name)
{
    return juce::Identifier(name);
}

juce::String stemGroupToString(StemGroup group)
{
    switch (group)
    {
        case StemGroup::fullMix: return "fullMix";
        case StemGroup::vocals: return "vocals";
        case StemGroup::drums: return "drums";
        case StemGroup::bass: return "bass";
        case StemGroup::other: return "other";
    }

    return "other";
}

StemGroup stemGroupFromString(const juce::String& value)
{
    if (value == "vocals")
        return StemGroup::vocals;
    if (value == "drums")
        return StemGroup::drums;
    if (value == "bass")
        return StemGroup::bass;
    if (value == "fullMix")
        return StemGroup::fullMix;

    return StemGroup::other;
}

juce::var makeJson(const GenerationPreset& preset)
{
    auto* object = new juce::DynamicObject();
    object->setProperty(idFor("schemaVersion"), kCurrentSchemaVersion);
    object->setProperty(idFor("id"), preset.id);
    object->setProperty(idFor("name"), preset.name);
    object->setProperty(idFor("prompt"), preset.request.prompt);
    object->setProperty(idFor("lyrics"), preset.request.lyrics);
    object->setProperty(idFor("durationSeconds"), preset.request.durationSeconds);
    object->setProperty(idFor("seed"), preset.request.seed);
    object->setProperty(idFor("cfgScale"), preset.request.cfgScale);
    object->setProperty(idFor("lmSeed"), preset.request.lmSeed);
    object->setProperty(idFor("scheduler"), preset.request.scheduler);
    object->setProperty(idFor("referenceAudioPath"),
        preset.request.referenceAudioPath.value_or(juce::String()));
    object->setProperty(idFor("stemsEnabled"), preset.request.stemsEnabled);
    object->setProperty(idFor("outputPath"), preset.request.outputPath);
    object->setProperty(idFor("midiExportRequested"), preset.midiExportRequested);
    object->setProperty(idFor("stemExportRequested"), preset.stemExportRequested);

    juce::Array<juce::var> groups;
    for (const auto group : preset.request.requestedStemGroups)
        groups.add(stemGroupToString(group));
    object->setProperty(idFor("requestedStemGroups"), juce::var(groups));

    return juce::var(object);
}

juce::String getString(juce::DynamicObject& object, const char* name)
{
    return object.getProperty(idFor(name)).toString();
}

GenerationPreset presetFromJson(juce::DynamicObject& object)
{
    GenerationPreset preset;
    preset.schemaVersion = object.getProperty(idFor("schemaVersion"));
    if (preset.schemaVersion <= 0)
        preset.schemaVersion = kCurrentSchemaVersion;

    preset.id = getString(object, "id");
    preset.name = getString(object, "name");
    preset.request.prompt = getString(object, "prompt");
    preset.request.lyrics = getString(object, "lyrics");
    preset.request.durationSeconds = static_cast<float>(
        static_cast<double>(object.getProperty(idFor("durationSeconds"))));
    if (preset.request.durationSeconds <= 0.0f)
        preset.request.durationSeconds = 30.0f;

    preset.request.seed = object.getProperty(idFor("seed"));
    preset.request.cfgScale = static_cast<float>(
        static_cast<double>(object.getProperty(idFor("cfgScale"))));
    if (preset.request.cfgScale <= 0.0f)
        preset.request.cfgScale = 7.0f;

    preset.request.lmSeed = object.getProperty(idFor("lmSeed"));
    preset.request.scheduler = getString(object, "scheduler");
    if (preset.request.scheduler.isEmpty())
        preset.request.scheduler = "euler";

    const auto referencePath = getString(object, "referenceAudioPath");
    if (referencePath.isNotEmpty())
        preset.request.referenceAudioPath = referencePath;

    preset.request.stemsEnabled = object.getProperty(idFor("stemsEnabled"));
    preset.request.outputPath = getString(object, "outputPath");
    preset.midiExportRequested = object.getProperty(idFor("midiExportRequested"));
    preset.stemExportRequested = object.getProperty(idFor("stemExportRequested"));

    if (const auto* groups = object.getProperty(idFor("requestedStemGroups")).getArray())
        for (const auto& group : *groups)
            preset.request.requestedStemGroups.push_back(stemGroupFromString(group.toString()));

    return preset;
}

} // namespace

PresetStore::PresetStore(juce::File presetDirectory)
    : directory(std::move(presetDirectory))
{
}

juce::File PresetStore::getDefaultPresetDirectory()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("ACE-Step")
        .getChildFile("Presets");
}

PresetOperationResult PresetStore::save(const GenerationPreset& preset) const
{
    if (const auto validation = validatePresetId(preset.id); !validation.success)
        return validation;

    if (!directory.createDirectory())
        return { false, "Could not create preset directory: " + directory.getFullPathName() };

    const auto destination = getPresetFile(preset.id);
    juce::TemporaryFile temporary(destination, juce::TemporaryFile::useHiddenFile);
    if (!temporary.getFile().replaceWithText(juce::JSON::toString(makeJson(preset), true)))
        return { false, "Could not write temporary preset file: "
            + temporary.getFile().getFullPathName() };

    if (!temporary.overwriteTargetFileWithTemporary())
        return { false, "Could not replace preset file: " + destination.getFullPathName() };

    return { true, {} };
}

PresetLoadResult PresetStore::load(const juce::String& presetId) const
{
    if (const auto validation = validatePresetId(presetId); !validation.success)
        return { false, {}, validation.errorMessage };

    const auto file = getPresetFile(presetId);
    if (!file.existsAsFile())
        return { false, {}, "Preset file does not exist: " + file.getFullPathName() };

    juce::var parsed;
    const auto parseResult = juce::JSON::parse(file.loadFileAsString(), parsed);
    if (parseResult.failed())
        return { false, {}, "Invalid preset JSON: " + parseResult.getErrorMessage() };

    auto* object = parsed.getDynamicObject();
    if (object == nullptr)
        return { false, {}, "Invalid preset JSON: " + file.getFullPathName() };

    auto preset = presetFromJson(*object);
    if (preset.id.isEmpty())
        preset.id = presetId;

    return { true, std::move(preset), {} };
}

PresetListResult PresetStore::list() const
{
    PresetListResult result { true, {}, {} };
    for (const auto& file : directory.findChildFiles(juce::File::findFiles, false, "*.json"))
    {
        auto loaded = load(file.getFileNameWithoutExtension());
        if (!loaded.success)
            return { false, {}, loaded.errorMessage };

        result.presets.push_back(std::move(loaded.preset));
    }

    return result;
}

PresetOperationResult PresetStore::rename(
    const juce::String& presetId, const juce::String& newName) const
{
    auto loaded = load(presetId);
    if (!loaded.success)
        return { false, loaded.errorMessage };

    loaded.preset.name = newName;
    return save(loaded.preset);
}

PresetOperationResult PresetStore::deletePreset(const juce::String& presetId) const
{
    if (const auto validation = validatePresetId(presetId); !validation.success)
        return validation;

    const auto file = getPresetFile(presetId);
    if (!file.existsAsFile())
        return { false, "Preset file does not exist: " + file.getFullPathName() };

    return file.deleteFile()
        ? PresetOperationResult { true, {} }
        : PresetOperationResult { false, "Could not delete preset file: " + file.getFullPathName() };
}

juce::File PresetStore::getPresetFile(const juce::String& presetId) const
{
    return directory.getChildFile(presetId + ".json");
}

PresetOperationResult PresetStore::validatePresetId(const juce::String& presetId) const
{
    if (presetId.isEmpty())
        return { false, "Preset id is required" };

    for (const auto character : presetId)
        if (!juce::CharacterFunctions::isLetterOrDigit(character) && character != '-' && character != '_')
            return { false, "Preset id contains unsupported characters" };

    return { true, {} };
}

} // namespace acestep_plugin
