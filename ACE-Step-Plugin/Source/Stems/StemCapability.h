#pragma once

#include "StemTypes.h"

namespace acestep_plugin
{

/** Centralizes whether stem export is enabled for a generation result. */
class StemCapability final
{
public:
    static bool isAvailable(StemCapabilityState state) noexcept;

private:
    StemCapability() = delete;
};

} // namespace acestep_plugin
