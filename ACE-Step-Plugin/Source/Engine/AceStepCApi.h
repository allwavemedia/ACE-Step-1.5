#pragma once

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

struct BackendPaths
{
    juce::File bundleBinaryDirectory;
    juce::File modelsDirectory;
};

class AceStepCApi final
{
public:
    AceStepCApi() = default;

    static BackendPaths getDefaultBackendPaths();
    static bool initializeBundledBackends(const juce::File& bundleBinaryDirectory);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AceStepCApi)
};

} // namespace acestep_plugin
