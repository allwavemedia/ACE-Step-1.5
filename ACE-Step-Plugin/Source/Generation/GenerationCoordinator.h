/** Headless coordinator for prompt-to-WAV generation.
 *
 *  Accepts a GenerationFormState, builds and submits a SidecarJobRequest
 *  through an injected GenerationSidecarGateway, tracks lifecycle state,
 *  validates the result manifest, and promotes exactly one verified full-mix
 *  WAV into GeneratedAssetHistory.
 *
 *  Owns no UI and performs no model or audio work.  Must be driven from a
 *  background or message thread; never call from processBlock().
 */
#pragma once

#include "../Models/GeneratedAssetHistory.h"
#include "../Sidecar/SidecarNamedPipeClient.h"
#include "../Sidecar/SidecarProcess.h"
#include "../Sidecar/SidecarRequest.h"
#include "GenerationFormState.h"

#include <juce_core/juce_core.h>

#include <functional>

namespace acestep_plugin
{

/** Error codes returned by GenerationCoordinator::startGeneration(). */
enum class GenerationCoordinatorError
{
    none,
    invalidFormState,          /**< form.validate() returned a non-empty error. */
    capturedReferenceRequired, /**< useCapturedReference=true but no resolver supplied. */
    submitFailed,              /**< Sidecar gateway returned a non-none submit error. */
    alreadyRunning,            /**< A generation or cancellation is still in progress. */
};

/** Observable lifecycle status of the coordinator. */
enum class GenerationCoordinatorStatus
{
    idle,       /**< No active generation. */
    running,    /**< Generation submitted and awaiting completion. */
    cancelling, /**< Cancellation sent; awaiting final event. */
    succeeded,  /**< Generation completed and asset promoted. */
    failed,     /**< Generation failed; no asset promoted. */
    cancelled,  /**< Generation was cancelled; no asset promoted. */
};

/** Immutable snapshot of coordinator state for UI binding. */
struct GenerationCoordinatorState
{
    GenerationCoordinatorStatus status = GenerationCoordinatorStatus::idle;
    juce::String activeRequestId;
    float progressFraction = 0.0f;
    juce::String statusText;
    juce::String errorText;
};

// ---------------------------------------------------------------------------

/** Abstract gateway to sidecar generation operations.
 *
 *  Inject a FakeGateway in tests to avoid requiring a live named pipe.
 */
class GenerationSidecarGateway
{
public:
    virtual ~GenerationSidecarGateway() = default;

    /** Submit a generation job and block for the helper's acknowledgement. */
    virtual SubmitResult submitGeneration(const SidecarJobRequest& request) = 0;

    /** Poll once for a progress or completion event.
     *
     *  Writes into outEvent; outEvent.requestId will be empty if no message
     *  arrived within timeoutMs.  Returns a SidecarClientError on transport
     *  or correlation failure.
     */
    virtual SidecarClientError pollProgress(const juce::String& requestId,
                                            ProgressEvent& outEvent,
                                            int timeoutMs) = 0;

    /** Send a cooperative cancellation notification for the active request. */
    virtual SidecarClientError cancelGeneration(const juce::String& requestId) = 0;
};

// ---------------------------------------------------------------------------

/** Headless generation coordinator.
 *
 *  Drive the state machine from a timer callback or background worker via
 *  repeated pollOnce() calls.  Task 6 binds to getState() for UI updates.
 */
class GenerationCoordinator final
{
public:
    /** Creates and returns a per-job output directory on demand. */
    using DirectoryProvider = std::function<juce::File(const juce::String& prefix)>;

    /** Resolves the captured reference audio WAV path; returns empty when unavailable. */
    using ReferenceAudioResolver = std::function<juce::String()>;

    /** Construct with mandatory history reference and gateway reference.
     *
     *  @param history            History that receives promoted WAV assets.
     *  @param gateway            Injectable sidecar gateway.
     *  @param directoryProvider  Optional; defaults to SidecarPaths::createJobDirectory.
     *  @param referenceResolver  Required when any form may set useCapturedReference=true.
     */
    GenerationCoordinator(GeneratedAssetHistory& history,
                          GenerationSidecarGateway& gateway,
                          DirectoryProvider directoryProvider = nullptr,
                          ReferenceAudioResolver referenceResolver = nullptr);

    ~GenerationCoordinator() = default;

    /** Validate the form state, create a job directory, build and submit a
     *  SidecarJobRequest.  Transitions to running on success.
     *
     *  Returns:
     *  - none on success (status → running).
     *  - invalidFormState when form.validate() is non-empty.
     *  - capturedReferenceRequired when useCapturedReference=true and no resolver.
     *  - submitFailed when the gateway returns a non-none SubmitResult error.
     *  - alreadyRunning when status is running or cancelling.
     *
     *  On any non-none error the history is not modified.
     */
    GenerationCoordinatorError startGeneration(const GenerationFormState& state);

    /** Poll the gateway once and advance the state machine.
     *
     *  Call from a timer callback or background worker.  On a completion event,
     *  validates the manifest and (if valid) promotes exactly one asset to
     *  history.  Safe to call when idle or after terminal status.
     *
     *  @param pollTimeoutMs  Maximum time to spend inside the gateway poll call.
     */
    void pollOnce(int pollTimeoutMs = 100);

    /** Send a cancellation notification for the active request.
     *
     *  Transitions to cancelling; the next pollOnce() that receives a
     *  completion event will finalise to cancelled without promotion.
     *  No-op when not in the running state.
     */
    void cancelActiveGeneration();

    /** Return an immutable snapshot of the current coordinator state. */
    GenerationCoordinatorState getState() const;

private:
    juce::String buildParametersJson(const GenerationRequest& request) const;
    void promoteSuccessfulAsset(const GenerationFormState& form,
                                const juce::String& outputWavPath);

    GeneratedAssetHistory& _history;
    GenerationSidecarGateway& _gateway;
    DirectoryProvider _directoryProvider;
    ReferenceAudioResolver _referenceResolver;

    GenerationCoordinatorState _state;
    GenerationFormState _activeForm;
    juce::File _activeJobDir;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerationCoordinator)
};

} // namespace acestep_plugin
