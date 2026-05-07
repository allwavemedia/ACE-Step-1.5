#pragma once

#include "StemTypes.h"

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

/** Centralizes whether stem export is enabled for a generation result. */
class StemCapability final
{
public:
    static bool isAvailable(StemCapabilityState state) noexcept;
    static juce::String getDisplayName(StemGroup group);

private:
    StemCapability() = delete;
};

} // namespace acestep_plugin
