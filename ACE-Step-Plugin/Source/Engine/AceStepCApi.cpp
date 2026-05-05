#include "AceStepCApi.h"

#if JUCE_WINDOWS
#include <windows.h>
#endif

#if !ACESTEP_PLUGIN_MODE_STUB
extern "C" void ggml_backend_load_all_from_path(const char* dir_path);
#endif

namespace acestep_plugin
{

namespace
{

juce::File getCurrentModuleFile()
{
#if JUCE_WINDOWS
    HMODULE moduleHandle = nullptr;
    constexpr auto flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

    const auto address = reinterpret_cast<LPCWSTR>(&getCurrentModuleFile);
    if (::GetModuleHandleExW(flags, address, &moduleHandle) != 0)
    {
        wchar_t path[MAX_PATH] = {};
        const auto length = ::GetModuleFileNameW(moduleHandle, path, MAX_PATH);
        if (length > 0 && length < MAX_PATH)
            return juce::File(juce::String(path));
    }
#endif

    return juce::File::getSpecialLocation(juce::File::currentExecutableFile);
}

juce::File getLocalAppDataDirectory()
{
    const auto localAppData = juce::SystemStats::getEnvironmentVariable("LOCALAPPDATA", {});

    if (localAppData.isNotEmpty())
        return juce::File(localAppData);

    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
}

} // namespace

BackendPaths AceStepCApi::getDefaultBackendPaths()
{
    auto bundleBinaryDirectory = getCurrentModuleFile().getParentDirectory();

    auto modelsDirectory = getLocalAppDataDirectory()
        .getChildFile("AceStepPlugin")
        .getChildFile("models");

    return { bundleBinaryDirectory, modelsDirectory };
}

bool AceStepCApi::initializeBundledBackends(const juce::File& bundleBinaryDirectory)
{
#if ACESTEP_PLUGIN_MODE_STUB
    juce::ignoreUnused(bundleBinaryDirectory);
    return false;
#else
    if (!bundleBinaryDirectory.isDirectory())
        return false;

    const auto backendPath = bundleBinaryDirectory.getFullPathName();
    ggml_backend_load_all_from_path(backendPath.toRawUTF8());
    return true;
#endif
}

} // namespace acestep_plugin
