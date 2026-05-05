#include "ModelSetupState.h"

namespace acestep_plugin
{

ModelSetupState ModelSetupStateManager::getState() const
{
    const juce::ScopedLock sl(lock);
    return state;
}

void ModelSetupStateManager::setState(const ModelSetupState& newState)
{
    const juce::ScopedLock sl(lock);
    state = newState;
}

bool ModelSetupStateManager::isReady() const
{
    const juce::ScopedLock sl(lock);
    return state.phase == ModelSetupPhase::Ready;
}

} // namespace acestep_plugin
