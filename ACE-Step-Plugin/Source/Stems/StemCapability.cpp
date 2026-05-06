#include "StemCapability.h"

namespace acestep_plugin
{

bool StemCapability::isAvailable(StemCapabilityState state) noexcept
{
    return state == StemCapabilityState::available;
}

} // namespace acestep_plugin
