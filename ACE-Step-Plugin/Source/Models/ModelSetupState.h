#pragma once

#include <juce_core/juce_core.h>

namespace acestep_plugin
{

/** Lifecycle phase for the model setup workflow. */
enum class ModelSetupPhase
{
    Idle,        ///< No check has been performed yet.
    Checking,    ///< Verifying local model files.
    Downloading, ///< One or more models are being downloaded.
    Ready,       ///< All required models are present and verified.
    Failed,      ///< A download or verification step failed.
};

/** Value type describing the current state of model setup. */
struct ModelSetupState
{
    ModelSetupPhase phase = ModelSetupPhase::Idle;

    /** Overall download progress in [0, 1]. Valid during Downloading phase. */
    float overallProgress = 0.0f;

    /** Filename currently being downloaded; empty otherwise. */
    juce::String currentFilename;

    /** Human-readable error description; non-empty only in Failed phase. */
    juce::String errorMessage;
};

/** Thread-safe manager for the model setup workflow state.
 *
 *  Callers on any thread may read the current state; the background
 *  download worker writes it.
 */
class ModelSetupStateManager final
{
public:
    ModelSetupStateManager() = default;

    ModelSetupState getState() const;
    void setState(const ModelSetupState& newState);

    /** Convenience helper: return true iff phase == Ready. */
    bool isReady() const;

private:
    mutable juce::CriticalSection lock;
    ModelSetupState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelSetupStateManager)
};

} // namespace acestep_plugin
