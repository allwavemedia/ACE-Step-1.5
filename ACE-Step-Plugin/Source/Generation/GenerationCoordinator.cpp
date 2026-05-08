/** Headless coordinator for prompt-to-WAV generation.
 *
 *  Implements the lifecycle state machine described in GenerationCoordinator.h.
 *  All methods are designed to be driven from a background or message thread.
 */
#include "GenerationCoordinator.h"

#include "../Sidecar/SidecarManifest.h"
#include "../Sidecar/SidecarPaths.h"

#include <juce_core/juce_core.h>

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
    const auto currentStatus = _state.status;
    if (currentStatus == GenerationCoordinatorStatus::running
        || currentStatus == GenerationCoordinatorStatus::cancelling)
    {
        return GenerationCoordinatorError::alreadyRunning;
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

    const auto submitResult = _gateway.submitGeneration(sidecarRequest);

    if (submitResult.error != SidecarClientError::none)
    {
        _state.status = GenerationCoordinatorStatus::failed;
        _state.activeRequestId = {};
        _state.errorText = "Submit failed with error code "
                           + juce::String(static_cast<int>(submitResult.error));
        return GenerationCoordinatorError::submitFailed;
    }

    // Gap 1: verify the sidecar acknowledged the correct request ID.
    if (submitResult.acknowledgedRequestId != sidecarRequest.requestId)
    {
        _state.status = GenerationCoordinatorStatus::failed;
        _state.activeRequestId = {};
        _state.errorText = "Submit request ID mismatch: sent "
                           + sidecarRequest.requestId
                           + " but sidecar acknowledged "
                           + submitResult.acknowledgedRequestId;
        return GenerationCoordinatorError::submitFailed;
    }

    // Commit running state.
    _activeForm = state;
    _activeJobDir = jobDir;
    _state = {};
    _state.status = GenerationCoordinatorStatus::running;
    _state.activeRequestId = sidecarRequest.requestId;
    _state.progressFraction = 0.0f;

    return GenerationCoordinatorError::none;
}

void GenerationCoordinator::pollOnce(int pollTimeoutMs)
{
    if (_state.status != GenerationCoordinatorStatus::running
        && _state.status != GenerationCoordinatorStatus::cancelling)
    {
        return;
    }

    ProgressEvent event;
    const auto pollError = _gateway.pollProgress(_state.activeRequestId, event, pollTimeoutMs);

    if (pollError == SidecarClientError::helperDisconnected)
    {
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
        _state.status = GenerationCoordinatorStatus::failed;
        _state.errorText = "Poll error: " + juce::String(static_cast<int>(pollError));
        return;
    }

    // No message arrived within the timeout window — keep waiting.
    if (event.requestId.isEmpty())
        return;

    // Gap 2: discard events that don't belong to the active request.
    if (event.requestId != _state.activeRequestId)
        return;

    _state.progressFraction = event.progressFraction;
    _state.statusText = event.statusMessage;

    if (!event.isComplete)
        return;

    // Completion received.
    if (_state.status == GenerationCoordinatorStatus::cancelling)
    {
        _state.status = GenerationCoordinatorStatus::cancelled;
        return;
    }

    // Validate the manifest before promoting any asset.
    const auto manifestFile = _activeJobDir.getChildFile("manifest.json");
    const auto manifestResult = validateResultManifest(manifestFile, _state.activeRequestId);

    if (manifestResult != SidecarProcessError::none)
    {
        _state.status = GenerationCoordinatorStatus::failed;
        _state.errorText = "Manifest validation failed.";
        return;
    }

    const auto outputWavPath = _activeJobDir.getChildFile("full_mix.wav").getFullPathName();
    promoteSuccessfulAsset(_activeForm, outputWavPath);
    _state.status = GenerationCoordinatorStatus::succeeded;
}

void GenerationCoordinator::cancelActiveGeneration()
{
    if (_state.status != GenerationCoordinatorStatus::running)
        return;

    const auto cancelError = _gateway.cancelGeneration(_state.activeRequestId);

    // Gap 3: any transport/protocol error means we cannot expect a graceful cancel event;
    // transition directly to failed rather than leaving a permanent cancelling state.
    if (cancelError != SidecarClientError::none)
    {
        _state.status = GenerationCoordinatorStatus::failed;
        _state.errorText = "Cancel failed with error code "
                           + juce::String(static_cast<int>(cancelError));
        return;
    }

    _state.status = GenerationCoordinatorStatus::cancelling;
}

GenerationCoordinatorState GenerationCoordinator::getState() const
{
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
