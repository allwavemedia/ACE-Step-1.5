#include "StemCapability.h"

namespace acestep_plugin
{

bool StemCapability::isAvailable(StemCapabilityState state) noexcept
{
    return state == StemCapabilityState::available;
}

juce::String StemCapability::getDisplayName(StemGroup group)
{
    switch (group)
    {
        case StemGroup::fullMix: return "full mix";
        case StemGroup::vocals: return "vocals";
        case StemGroup::drums: return "drums";
        case StemGroup::bass: return "bass";
        case StemGroup::other: return "other";
    }

    return "stem";
}

} // namespace acestep_plugin
