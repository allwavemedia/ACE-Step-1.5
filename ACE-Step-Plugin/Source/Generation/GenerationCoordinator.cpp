/** Headless coordinator for prompt-to-WAV generation.
 *
 *  Implements the lifecycle state machine described in GenerationCoordinator.h.
 *  All methods are designed to be driven from a background or message thread.
 */
#include "GenerationCoordinator.h"

#include "../Sidecar/SidecarManifest.h"
#include "../Sidecar/SidecarPaths.h"

#include <juce_core/juce_core.h>

#include <mutex>

namespace acestep_plugin
{

GenerationCoordinator::GenerationCoordinator(GeneratedAssetHistory& history,
                                             GenerationSidecarGateway& gateway,
                                             DirectoryProvider directoryProvider,
                                             ReferenceAudioResolver referenceResolver)
    : _history(history)
    , _gateway(gateway)
    , _directoryProvider(std::move(directoryProvider))
    , _referenceResolver(std::move(referenceResolver))
{
}

GenerationCoordinatorError GenerationCoordinator::startGeneration(const GenerationFormState& state)
{
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        const auto currentStatus = _state.status;
        if (currentStatus == GenerationCoordinatorStatus::running
            || currentStatus == GenerationCoordinatorStatus::cancelling)
        {
            return GenerationCoordinatorError::alreadyRunning;
        }
    }

    const auto validationError = state.validate();
    if (validationError.isNotEmpty())
        return GenerationCoordinatorError::invalidFormState;

    if (state.useCapturedReference && _referenceResolver == nullptr)
        return GenerationCoordinatorError::capturedReferenceRequired;

    // Create the output directory for this job.
    juce::File jobDir;
    if (_directoryProvider != nullptr)
        jobDir = _directoryProvider("generation");
    else
        jobDir = SidecarPaths::createJobDirectory("generation");

    const auto outputWavPath = jobDir.getChildFile("full_mix.wav").getFullPathName();

    // Build the generation request to populate parametersJson.
    GenerationRequest genRequest = state.toRequest(outputWavPath);

    if (state.useCapturedReference)
        genRequest.referenceAudioPath = _referenceResolver();

    // Build the sidecar wire envelope.
    SidecarJobRequest sidecarRequest;
    sidecarRequest.protocolVersion = "1.0";
    sidecarRequest.requestId = juce::Uuid().toString();
    sidecarRequest.sessionId = juce::Uuid().toString();
    sidecarRequest.jobKind = SidecarJobKind::generation;
    sidecarRequest.outputDirectory = jobDir.getFullPathName();
    sidecarRequest.parametersJson = buildParametersJson(genRequest);

    // Submit to the gateway (outside lock — may block on I/O).
    const auto submitResult = _gateway.submitGeneration(sidecarRequest);

    if (submitResult.error != SidecarClientError::none)
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _state.status = GenerationCoordinatorStatus::failed;
        _state.activeRequestId = {};
        _state.errorText = "Submit failed with error code "
                           + juce::String(static_cast<int>(submitResult.error));
        return GenerationCoordinatorError::submitFailed;
    }

    // Verify the sidecar acknowledged the correct request ID.
    if (submitResult.acknowledgedRequestId != sidecarRequest.requestId)
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _state.status = GenerationCoordinatorStatus::failed;
        _state.activeRequestId = {};
        _state.errorText = "Submit request ID mismatch: sent "
                           + sidecarRequest.requestId
                           + " but sidecar acknowledged "
                           + submitResult.acknowledgedRequestId;
        return GenerationCoordinatorError::submitFailed;
    }

    // Commit running state — write _activeForm/_activeJobDir then update _state.
    _activeForm = state;
    _activeJobDir = jobDir;

    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _state = {};
        _state.status = GenerationCoordinatorStatus::running;
        _state.activeRequestId = sidecarRequest.requestId;
        _state.progressFraction = 0.0f;
    }

    return GenerationCoordinatorError::none;
}

void GenerationCoordinator::pollOnce(int pollTimeoutMs)
{
    // Snapshot the fields needed for the gateway call (outside lock scope).
    GenerationCoordinatorStatus currentStatus;
    juce::String currentRequestId;
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        currentStatus = _state.status;
        currentRequestId = _state.activeRequestId;
    }

    if (currentStatus != GenerationCoordinatorStatus::running
        && currentStatus != GenerationCoordinatorStatus::cancelling)
    {
        return;
    }

    // Poll the gateway without holding the mutex (may block on I/O).
    ProgressEvent event;
    const auto pollError = _gateway.pollProgress(currentRequestId, event, pollTimeoutMs);

    if (pollError == SidecarClientError::helperDisconnected)
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _state.status = GenerationCoordinatorStatus::failed;
        _state.errorText = "Helper disconnected during generation.";
        return;
    }

    // staleCompletion signals a completion for a cancelled/superseded request.
    // The active job should continue; treat it like receiving no meaningful event.
    if (pollError == SidecarClientError::staleCompletion)
        return;

    if (pollError != SidecarClientError::none)
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _state.status = GenerationCoordinatorStatus::failed;
        _state.errorText = "Poll error: " + juce::String(static_cast<int>(pollError));
        return;
    }

    // No message arrived within the timeout window — keep waiting.
    if (event.requestId.isEmpty())
        return;

    // Discard events that don't belong to the active request.
    if (event.requestId != currentRequestId)
        return;

    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _state.progressFraction = event.progressFraction;
        _state.statusText = event.statusMessage;
    }

    if (!event.isComplete)
        return;

    // Completion received — check current status under lock in case a concurrent
    // cancelActiveGeneration() changed it while the gateway call was in flight.
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        if (_state.status == GenerationCoordinatorStatus::cancelling)
        {
            _state.status = GenerationCoordinatorStatus::cancelled;
            return;
        }
    }

    // Validate the manifest and retrieve artifact paths in one pass (outside lock —
    // filesystem + SHA-256 work should not hold the mutex).
    const auto manifestFile = _activeJobDir.getChildFile("manifest.json");
    const auto artifactPaths = getValidatedArtifactPaths(manifestFile, currentRequestId);

    if (artifactPaths.isEmpty())
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _state.status = GenerationCoordinatorStatus::failed;
        _state.errorText = "Manifest validation failed.";
        return;
    }

    // Promote the first validated artifact from the manifest — never hardcoded path.
    promoteSuccessfulAsset(_activeForm, artifactPaths[0]);

    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        _state.status = GenerationCoordinatorStatus::succeeded;
    }
}

void GenerationCoordinator::cancelActiveGeneration()
{
    juce::String requestId;
    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        if (_state.status != GenerationCoordinatorStatus::running)
            return;
        requestId = _state.activeRequestId;
    }

    // Send cancellation outside lock (may block on I/O).
    const auto cancelError = _gateway.cancelGeneration(requestId);

    {
        std::lock_guard<std::mutex> lock(_stateMutex);
        // Any transport/protocol error means we cannot expect a graceful cancel event;
        // transition directly to failed rather than leaving a permanent cancelling state.
        if (cancelError != SidecarClientError::none)
        {
            _state.status = GenerationCoordinatorStatus::failed;
            _state.errorText = "Cancel failed with error code "
                               + juce::String(static_cast<int>(cancelError));
        }
        else
        {
            _state.status = GenerationCoordinatorStatus::cancelling;
        }
    }
}

GenerationCoordinatorState GenerationCoordinator::getState() const
{
    std::lock_guard<std::mutex> lock(_stateMutex);
    return _state;
}

// ---------------------------------------------------------------------------
// Private

juce::String GenerationCoordinator::buildParametersJson(const GenerationRequest& request) const
{
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("prompt", request.prompt);
    obj->setProperty("lyrics", request.lyrics);
    obj->setProperty("durationSeconds", static_cast<double>(request.durationSeconds));
    obj->setProperty("seed", request.seed);
    obj->setProperty("cfgScale", static_cast<double>(request.cfgScale));
    obj->setProperty("lmSeed", request.lmSeed);
    obj->setProperty("scheduler", request.scheduler);
    obj->setProperty("outputPath", request.outputPath);

    if (request.referenceAudioPath.has_value() && !request.referenceAudioPath->isEmpty())
        obj->setProperty("referenceAudioPath", *request.referenceAudioPath);

    return juce::JSON::toString(juce::var(obj.get()));
}

void GenerationCoordinator::promoteSuccessfulAsset(const GenerationFormState& form,
                                                   const juce::String& outputWavPath)
{
    GeneratedAsset asset;
    asset.id = juce::Uuid().toString();
    asset.outputPath = outputWavPath;
    asset.durationSeconds = form.durationSeconds;
    asset.timestamp = juce::Time::getCurrentTime();
    asset.midiAvailability = MidiExportAvailability::unavailable;

    _history.add(asset);
}

} // namespace acestep_plugin
