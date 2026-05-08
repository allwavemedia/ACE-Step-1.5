/** Behaviour tests for GenerationCoordinator.
 *
 *  Uses a fake gateway (no real named pipe) and real validateResultManifest
 *  for manifest validation cases.  Covers successful promotion, invalid form
 *  state, submit failure, bad manifest, cancellation, history preservation,
 *  and parametersJson field content.
 */
#include "../Source/Generation/GenerationCoordinator.h"

#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>

#include <atomic>
#include <chrono>
#include <deque>
#include <optional>
#include <thread>

namespace acestep_plugin
{

// ---------------------------------------------------------------------------
// Fake gateway

class FakeGateway final : public GenerationSidecarGateway
{
public:
    struct PollResponse
    {
        SidecarClientError error = SidecarClientError::none;
        ProgressEvent event;
    };

    SidecarClientError submitError = SidecarClientError::none;
    /** When set, returned as acknowledgedRequestId even if submitError == none (Gap 1). */
    std::optional<juce::String> submitAcknowledgedRequestIdOverride;
    /** When set, returned by cancelGeneration instead of none (Gap 3). */
    SidecarClientError cancelError = SidecarClientError::none;
    std::deque<PollResponse> pollResponses;
    juce::String lastCancelledRequestId;
    std::optional<SidecarJobRequest> lastSubmittedRequest;
    std::atomic<bool> blockPoll { false };
    std::atomic<bool> pollEntered { false };
    std::atomic<bool> releasePoll { false };
    std::atomic<bool> blockCancel { false };
    std::atomic<bool> cancelEntered { false };
    std::atomic<bool> releaseCancel { false };

    SubmitResult submitGeneration(const SidecarJobRequest& request) override
    {
        lastSubmittedRequest = request;
        SubmitResult result;
        result.error = submitError;
        if (submitError == SidecarClientError::none)
        {
            result.acknowledgedRequestId =
                submitAcknowledgedRequestIdOverride.value_or(request.requestId);
            result.artifactRoot = request.outputDirectory;
        }
        return result;
    }

    SidecarClientError pollProgress(const juce::String& /*requestId*/,
                                     ProgressEvent& outEvent,
                                     int /*timeoutMs*/) override
    {
        pollEntered = true;
        while (blockPoll && !releasePoll)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (pollResponses.empty())
        {
            outEvent = {};
            return SidecarClientError::none;
        }
        auto r = pollResponses.front();
        pollResponses.pop_front();
        outEvent = r.event;
        return r.error;
    }

    SidecarClientError cancelGeneration(const juce::String& requestId) override
    {
        lastCancelledRequestId = requestId;
        cancelEntered = true;
        while (blockCancel && !releaseCancel)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        return cancelError;
    }
};

// ---------------------------------------------------------------------------
// Helpers

static juce::File makeCleanTestDirectory(const juce::String& name)
{
    const auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                         .getChildFile(name);
    dir.deleteRecursively();
    dir.createDirectory();
    return dir;
}

/** Write a valid manifest + artifact file in jobDir for requestId. */
static void writeMinimalWavFile(const juce::File& artFile)
{
    const unsigned char wavBytes[] = {
        'R', 'I', 'F', 'F', 38, 0, 0, 0, 'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 1, 0,
        0x44, 0xac, 0, 0, 0x88, 0x58, 0x01, 0, 2, 0, 16, 0,
        'd', 'a', 't', 'a', 2, 0, 0, 0, 0, 0
    };

    artFile.replaceWithData(wavBytes, sizeof(wavBytes));
}

static void writeValidManifest(const juce::File& jobDir,
                                 const juce::String& requestId,
                                 const juce::String& artifactName = "full_mix.wav")
{
    const auto artFile = jobDir.getChildFile(artifactName);
    if (artFile.hasFileExtension("wav"))
        writeMinimalWavFile(artFile);
    else
        artFile.replaceWithText("FAKE_AUDIO_CONTENT");

    const auto sha256 = juce::SHA256(artFile).toHexString();

    juce::DynamicObject::Ptr artObj = new juce::DynamicObject();
    artObj->setProperty("path", artFile.getFullPathName());
    artObj->setProperty("byteSize", static_cast<juce::int64>(artFile.getSize()));
    artObj->setProperty("sha256", sha256);

    juce::Array<juce::var> artArray;
    artArray.add(juce::var(artObj.get()));

    juce::DynamicObject::Ptr manifest = new juce::DynamicObject();
    manifest->setProperty("protocolVersion", juce::String("1.0"));
    manifest->setProperty("requestId", requestId);
    manifest->setProperty("success", true);
    manifest->setProperty("artifacts", juce::var(artArray));

    jobDir.getChildFile("manifest.json")
        .replaceWithText(juce::JSON::toString(juce::var(manifest.get())));
}

/** Write a manifest for a malformed .wav file whose metadata and hash match. */
static void writeMalformedWavManifest(const juce::File& jobDir,
                                      const juce::String& requestId)
{
    const auto artFile = jobDir.getChildFile("full_mix.wav");
    const unsigned char badRiffSizeWav[] = {
        'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E',
        'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 1, 0,
        0x44, 0xac, 0, 0, 0x88, 0x58, 0x01, 0, 2, 0, 16, 0,
        'd', 'a', 't', 'a', 2, 0, 0, 0, 0, 0
    };
    artFile.replaceWithData(badRiffSizeWav, sizeof(badRiffSizeWav));

    juce::DynamicObject::Ptr artObj = new juce::DynamicObject();
    artObj->setProperty("path", artFile.getFullPathName());
    artObj->setProperty("byteSize", static_cast<juce::int64>(artFile.getSize()));
    artObj->setProperty("sha256", juce::SHA256(artFile).toHexString());

    juce::Array<juce::var> artArray;
    artArray.add(juce::var(artObj.get()));

    juce::DynamicObject::Ptr manifest = new juce::DynamicObject();
    manifest->setProperty("protocolVersion", juce::String("1.0"));
    manifest->setProperty("requestId", requestId);
    manifest->setProperty("success", true);
    manifest->setProperty("artifacts", juce::var(artArray));

    jobDir.getChildFile("manifest.json")
        .replaceWithText(juce::JSON::toString(juce::var(manifest.get())));
}

/** Write a manifest containing multiple valid artifact entries. */
static void writeMultiArtifactManifest(const juce::File& jobDir,
                                       const juce::String& requestId)
{
    juce::Array<juce::var> artArray;

    for (const auto& artifactName : { juce::String("full_mix.wav"),
                                      juce::String("extra.wav") })
    {
        const auto artFile = jobDir.getChildFile(artifactName);
        artFile.replaceWithText("FAKE_AUDIO_CONTENT_" + artifactName);

        juce::DynamicObject::Ptr artObj = new juce::DynamicObject();
        artObj->setProperty("path", artFile.getFullPathName());
        artObj->setProperty("byteSize", static_cast<juce::int64>(artFile.getSize()));
        artObj->setProperty("sha256", juce::SHA256(artFile).toHexString());
        artArray.add(juce::var(artObj.get()));
    }

    juce::DynamicObject::Ptr manifest = new juce::DynamicObject();
    manifest->setProperty("protocolVersion", juce::String("1.0"));
    manifest->setProperty("requestId", requestId);
    manifest->setProperty("success", true);
    manifest->setProperty("artifacts", juce::var(artArray));

    jobDir.getChildFile("manifest.json")
        .replaceWithText(juce::JSON::toString(juce::var(manifest.get())));
}

/** Build a valid GenerationFormState suitable for tests. */
static GenerationFormState makeValidForm()
{
    GenerationFormState form;
    form.prompt = "dark ambient electronica";
    form.lyrics = "test lyrics";
    form.durationSeconds = 15.0f;
    form.seed = 42;
    form.cfgScale = 7.0f;
    form.lmSeed = 123;
    form.scheduler = "euler";
    form.useCapturedReference = false;
    return form;
}

/** Build a completion ProgressEvent for a given requestId. */
static ProgressEvent makeCompletionEvent(const juce::String& requestId)
{
    ProgressEvent e;
    e.requestId = requestId;
    e.progressFraction = 1.0f;
    e.statusMessage = "complete";
    e.isComplete = true;
    return e;
}

// ---------------------------------------------------------------------------
// Tests

class GenerationCoordinatorTests final : public juce::UnitTest
{
public:
    GenerationCoordinatorTests() : juce::UnitTest("GenerationCoordinator") {}

    void runTest() override
    {
        // ------------------------------------------------------------------
        beginTest("successful generation promotes exactly one verified WAV asset");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_success");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            const auto form = makeValidForm();
            const auto err = coordinator.startGeneration(form);
            expect(err == GenerationCoordinatorError::none, "startGeneration should succeed");

            const auto runState = coordinator.getState();
            expect(runState.status == GenerationCoordinatorStatus::running,
                   "should be running after startGeneration");

            // Write a valid manifest so validation will pass
            writeValidManifest(jobDir, runState.activeRequestId);

            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(runState.activeRequestId)});
            coordinator.pollOnce(0);

            const auto finalState = coordinator.getState();
            expect(finalState.status == GenerationCoordinatorStatus::succeeded,
                   "status should be succeeded after valid manifest");
            expectEquals(history.size(), 1, "exactly one asset promoted to history");

            const auto assets = history.getAssets();
            expect(assets[0].outputPath.isNotEmpty(), "outputPath must be non-empty");
            expectEquals(assets[0].durationSeconds, form.durationSeconds,
                         "durationSeconds must match form");
            expect(assets[0].midiAvailability == MidiExportAvailability::unavailable,
                   "midiAvailability must be unavailable");
            expect(assets[0].stems.empty(), "stems must be empty");
            expect(assets[0].id.isNotEmpty(), "asset id must be set");

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        beginTest("invalid form state fails before sidecar submit and does not add history");
        {
            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{history, gateway};

            GenerationFormState badForm;
            badForm.prompt = ""; // invalid: empty prompt
            badForm.durationSeconds = 30.0f;

            const auto err = coordinator.startGeneration(badForm);
            expect(err == GenerationCoordinatorError::invalidFormState,
                   "empty prompt should yield invalidFormState");
            expect(!gateway.lastSubmittedRequest.has_value(),
                   "gateway must not be called with invalid form state");
            expectEquals(history.size(), 0, "no history entry on invalid form");

            const auto state = coordinator.getState();
            expect(state.status == GenerationCoordinatorStatus::idle,
                   "status must remain idle after validation failure");
        }

        // ------------------------------------------------------------------
        beginTest("submit error causes failed state and does not add history");
        {
            GeneratedAssetHistory history;
            FakeGateway gateway;
            gateway.submitError = SidecarClientError::helperDisconnected;
            GenerationCoordinator coordinator{history, gateway};

            const auto err = coordinator.startGeneration(makeValidForm());
            expect(err == GenerationCoordinatorError::submitFailed,
                   "helper-disconnected submit should yield submitFailed");
            expectEquals(history.size(), 0, "no history on submit failure");

            const auto state = coordinator.getState();
            expect(state.status == GenerationCoordinatorStatus::failed,
                   "status should be failed after submit error");
            expect(state.errorText.isNotEmpty(), "errorText must be set on failure");
            // ITEM 2: activeRequestId must not be stale after submit failure.
            expect(state.activeRequestId.isEmpty(),
                   "activeRequestId must be empty after submit failure");
        }

        // ------------------------------------------------------------------
        beginTest("completion with corrupt manifest does not add history");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_badmanifest");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            const auto state = coordinator.getState();

            // Write syntactically corrupt JSON
            jobDir.getChildFile("manifest.json").replaceWithText("{NOT_VALID{{");

            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(state.activeRequestId)});
            coordinator.pollOnce(0);

            expectEquals(history.size(), 0, "corrupt manifest must not promote history");
            expect(coordinator.getState().status == GenerationCoordinatorStatus::failed,
                   "bad manifest must yield failed status");
            expect(coordinator.getState().errorText.isNotEmpty(),
                   "errorText must be set after manifest failure");

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        beginTest("completion with missing manifest does not add history");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_nomanifest");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            const auto state = coordinator.getState();

            // No manifest written — leave dir empty
            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(state.activeRequestId)});
            coordinator.pollOnce(0);

            expectEquals(history.size(), 0, "missing manifest must not promote history");
            expect(coordinator.getState().status == GenerationCoordinatorStatus::failed);

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        beginTest("completion with mismatched manifest requestId does not add history");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_badreqid");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            const auto state = coordinator.getState();

            // Write manifest with a different request ID
            writeValidManifest(jobDir, "WRONG-ID-THAT-DOES-NOT-MATCH");

            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(state.activeRequestId)});
            coordinator.pollOnce(0);

            expectEquals(history.size(), 0, "mismatched manifest requestId must not promote");
            expect(coordinator.getState().status == GenerationCoordinatorStatus::failed);

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        beginTest("cancellation sends cancel for active request and does not promote");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_cancel");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            const auto runState = coordinator.getState();
            expect(runState.status == GenerationCoordinatorStatus::running);

            coordinator.cancelActiveGeneration();
            expect(gateway.lastCancelledRequestId == runState.activeRequestId,
                   "cancelGeneration must be called with the active request ID");

            // Feed a late completion with a valid manifest — must not promote
            writeValidManifest(jobDir, runState.activeRequestId);
            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(runState.activeRequestId)});
            coordinator.pollOnce(0);

            expectEquals(history.size(), 0, "cancelled generation must not promote any asset");
            expect(coordinator.getState().status == GenerationCoordinatorStatus::cancelled,
                   "status should be cancelled");

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        beginTest("failed later generation preserves earlier successful asset in history");
        {
            const auto jobDir1 = makeCleanTestDirectory("gc_test_preserve1");
            const auto jobDir2 = makeCleanTestDirectory("gc_test_preserve2");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            int callCount = 0;
            GenerationCoordinator coordinator{
                history, gateway,
                [&](const juce::String&) { return ++callCount == 1 ? jobDir1 : jobDir2; }};

            // First generation: success
            coordinator.startGeneration(makeValidForm());
            const auto req1Id = coordinator.getState().activeRequestId;
            writeValidManifest(jobDir1, req1Id);
            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(req1Id)});
            coordinator.pollOnce(0);
            expectEquals(history.size(), 1, "first generation should add one asset");

            // Second generation: submit fails
            gateway.submitError = SidecarClientError::helperDisconnected;
            coordinator.startGeneration(makeValidForm());

            expectEquals(history.size(), 1, "failed generation must not remove earlier asset");
            expect(history.getAssets()[0].outputPath.isNotEmpty(),
                   "first asset still present and has non-empty outputPath");

            jobDir1.deleteRecursively();
            jobDir2.deleteRecursively();
        }

        // ------------------------------------------------------------------
        beginTest("generated parametersJson contains all required fields");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_params");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            GenerationFormState form;
            form.prompt = "test prompt value";
            form.lyrics = "la la la";
            form.durationSeconds = 20.0f;
            form.seed = 99;
            form.cfgScale = 5.5f;
            form.lmSeed = 77;
            form.scheduler = "dpmpp_2m";
            form.useCapturedReference = false;

            coordinator.startGeneration(form);

            expect(gateway.lastSubmittedRequest.has_value(),
                   "gateway must have received a submit request");
            const auto& req = *gateway.lastSubmittedRequest;
            expect(req.jobKind == SidecarJobKind::generation,
                   "jobKind must be generation");

            juce::var params;
            const auto parseResult = juce::JSON::parse(req.parametersJson, params);
            expect(parseResult.wasOk(), "parametersJson must be valid JSON");

            const auto* obj = params.getDynamicObject();
            expect(obj != nullptr, "parametersJson root must be a JSON object");

            if (obj != nullptr)
            {
                expectEquals(obj->getProperty("prompt").toString(),
                             juce::String("test prompt value"));
                expectEquals(obj->getProperty("lyrics").toString(), juce::String("la la la"));
                expect(std::abs(static_cast<float>(obj->getProperty("durationSeconds")) - 20.0f)
                           < 0.001f,
                       "durationSeconds must match");
                expectEquals(static_cast<int>(obj->getProperty("seed")), 99);
                expect(std::abs(static_cast<float>(obj->getProperty("cfgScale")) - 5.5f) < 0.001f,
                       "cfgScale must match");
                expectEquals(static_cast<int>(obj->getProperty("lmSeed")), 77);
                expectEquals(obj->getProperty("scheduler").toString(), juce::String("dpmpp_2m"));
                expect(obj->getProperty("outputPath").toString().isNotEmpty(),
                       "outputPath must be present and non-empty");
            }

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        beginTest("useCapturedReference without resolver returns error before submit");
        {
            GeneratedAssetHistory history;
            FakeGateway gateway;
            // No referenceResolver supplied
            GenerationCoordinator coordinator{history, gateway};

            GenerationFormState form = makeValidForm();
            form.useCapturedReference = true;

            const auto err = coordinator.startGeneration(form);
            expect(err == GenerationCoordinatorError::capturedReferenceRequired,
                   "should fail with capturedReferenceRequired");
            expect(!gateway.lastSubmittedRequest.has_value(),
                   "gateway must not be called when reference cannot be resolved");
            expectEquals(history.size(), 0, "no history on reference-required error");
        }

        // ------------------------------------------------------------------
        beginTest("alreadyRunning error returned when startGeneration called while running");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_already");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            const auto first = coordinator.startGeneration(makeValidForm());
            expect(first == GenerationCoordinatorError::none, "first start should succeed");

            const auto second = coordinator.startGeneration(makeValidForm());
            expect(second == GenerationCoordinatorError::alreadyRunning,
                   "second start while running should return alreadyRunning");

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        // GAP 1: submit returns error==none but wrong acknowledgedRequestId
        beginTest("submit ack requestId mismatch transitions to failed with mismatch error text");
        {
            GeneratedAssetHistory history;
            FakeGateway gateway;
            gateway.submitAcknowledgedRequestIdOverride = "WRONG-ACK-ID-XYZ";
            GenerationCoordinator coordinator{history, gateway};

            const auto err = coordinator.startGeneration(makeValidForm());
            expect(err == GenerationCoordinatorError::submitFailed,
                   "mismatched acknowledgedRequestId should yield submitFailed");
            expectEquals(history.size(), 0, "no history entry on ack mismatch");

            const auto state = coordinator.getState();
            expect(state.status == GenerationCoordinatorStatus::failed,
                   "status must be failed after ack mismatch");
            expect(state.errorText.isNotEmpty(),
                   "errorText must describe the request ID mismatch");
            // The error text should mention the mismatch explicitly
            const auto lowerError = state.errorText.toLowerCase();
            expect(lowerError.contains("mismatch") || lowerError.contains("request"),
                   "errorText should reference the request mismatch");
            // Active request ID must not be stale after submit failure (Gap 4)
            expect(state.activeRequestId.isEmpty(),
                   "activeRequestId must be empty after submit-phase failure");
        }

        // ------------------------------------------------------------------
        // GAP 2: pollOnce ignores progress event from stale/different requestId
        beginTest("pollOnce ignores progress event with stale requestId");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_stale_event");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());

            // Feed a progress event carrying a different (stale) requestId
            ProgressEvent staleEvent;
            staleEvent.requestId = "STALE-REQUEST-ID-99999";
            staleEvent.progressFraction = 0.75f;
            staleEvent.statusMessage = "stale progress message should be ignored";
            staleEvent.isComplete = false;

            gateway.pollResponses.push_back({SidecarClientError::none, staleEvent});
            coordinator.pollOnce(0);

            const auto state = coordinator.getState();
            expect(state.status == GenerationCoordinatorStatus::running,
                   "status must remain running after stale event");
            expect(state.progressFraction < 0.01f,
                   "progressFraction must not be updated from stale event");
            expect(state.statusText != staleEvent.statusMessage,
                   "statusText must not be updated from stale event");

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        // GAP 3: cancelActiveGeneration propagates helperDisconnected as failed
        beginTest("cancelActiveGeneration with helperDisconnected error transitions to failed");
        {
            GeneratedAssetHistory history;
            FakeGateway gateway;
            gateway.cancelError = SidecarClientError::helperDisconnected;
            GenerationCoordinator coordinator{history, gateway};

            coordinator.startGeneration(makeValidForm());
            expect(coordinator.getState().status == GenerationCoordinatorStatus::running,
                   "precondition: should be running");

            coordinator.cancelActiveGeneration();

            const auto state = coordinator.getState();
            expect(state.status == GenerationCoordinatorStatus::failed,
                   "helperDisconnected on cancel must transition to failed, not cancelling");
            expect(state.errorText.isNotEmpty(),
                   "errorText must be set when cancel returns helperDisconnected");
        }

        // ------------------------------------------------------------------
        // ITEM 1: cancelActiveGeneration must treat ANY non-none SidecarClientError as failed.
        beginTest("cancelActiveGeneration with any non-none error transitions to failed not cancelling");
        {
            GeneratedAssetHistory history;
            FakeGateway gateway;
            gateway.cancelError = SidecarClientError::unknownMessageType;
            GenerationCoordinator coordinator{history, gateway};

            coordinator.startGeneration(makeValidForm());
            expect(coordinator.getState().status == GenerationCoordinatorStatus::running,
                   "precondition: should be running");

            coordinator.cancelActiveGeneration();

            const auto state = coordinator.getState();
            expect(state.status == GenerationCoordinatorStatus::failed,
                   "unknownMessageType on cancel must transition to failed, not cancelling");
            expect(state.errorText.isNotEmpty(),
                   "errorText must be set when cancel returns non-none error");
            expect(state.status != GenerationCoordinatorStatus::cancelling,
                   "must not remain in cancelling state after a cancel error");
        }

        // ------------------------------------------------------------------
        // ITEM 3 (optional): pollOnce must ignore staleCompletion without failing the active job.
        beginTest("pollOnce with staleCompletion error keeps job running without failing");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_stale_completion");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            expect(coordinator.getState().status == GenerationCoordinatorStatus::running,
                   "precondition: should be running");

            // staleCompletion signals a completion for a cancelled/superseded request;
            // the active job must remain running, not fail.
            gateway.pollResponses.push_back({SidecarClientError::staleCompletion, {}});
            coordinator.pollOnce(0);

            const auto state = coordinator.getState();
            expect(state.status == GenerationCoordinatorStatus::running,
                   "staleCompletion must not fail or terminate the active job");
            expectEquals(history.size(), 0, "no asset promoted when staleCompletion received");

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        // HIGH 2: manifest-listed artifact path is used for promotion,
        // not the hardcoded full_mix.wav path.
        beginTest("coordinator promotes manifest-listed artifact path not hardcoded full_mix.wav");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_manifest_path");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            const auto form = makeValidForm();
            const auto startErr = coordinator.startGeneration(form);
            expect(startErr == GenerationCoordinatorError::none, "startGeneration should succeed");

            const auto runState = coordinator.getState();

            // Write a manifest with a DIFFERENT artifact name — NOT full_mix.wav.
            // full_mix.wav must not exist so the test fails if coordinator falls back
            // to the hardcoded path.
            writeValidManifest(jobDir, runState.activeRequestId, "custom_output.wav");
            expect(!jobDir.getChildFile("full_mix.wav").existsAsFile(),
                   "full_mix.wav must not exist for this test to be meaningful");

            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(runState.activeRequestId)});
            coordinator.pollOnce(0);

            const auto finalState = coordinator.getState();
            expect(finalState.status == GenerationCoordinatorStatus::succeeded,
                   "status should be succeeded when manifest lists a valid artifact");
            expectEquals(history.size(), 1, "exactly one asset promoted to history");

            const auto assets = history.getAssets();
            expect(assets[0].outputPath.contains("custom_output.wav"),
                   "promoted path must come from the manifest artifact, not full_mix.wav");

            jobDir.deleteRecursively();
        }

        beginTest("coordinator rejects non-WAV generation artifact");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_nonwav_artifact");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            const auto runState = coordinator.getState();

            writeValidManifest(jobDir, runState.activeRequestId, "not_audio.txt");

            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(runState.activeRequestId)});
            coordinator.pollOnce(0);

            expectEquals(history.size(), 0, "non-WAV generation artifact must not promote");
            expect(coordinator.getState().status == GenerationCoordinatorStatus::failed,
                   "non-WAV generation artifact must fail the coordinator");

            jobDir.deleteRecursively();
        }

        beginTest("coordinator rejects malformed WAV generation artifact");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_malformed_wav");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            const auto runState = coordinator.getState();

            writeMalformedWavManifest(jobDir, runState.activeRequestId);

            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(runState.activeRequestId)});
            coordinator.pollOnce(0);

            expectEquals(history.size(), 0, "malformed WAV artifact must not promote");
            expect(coordinator.getState().status == GenerationCoordinatorStatus::failed,
                   "malformed WAV artifact must fail the coordinator");

            jobDir.deleteRecursively();
        }

        beginTest("coordinator rejects manifest with more than one generation artifact");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_multi_artifact");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            const auto runState = coordinator.getState();

            writeMultiArtifactManifest(jobDir, runState.activeRequestId);

            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(runState.activeRequestId)});
            coordinator.pollOnce(0);

            expectEquals(history.size(), 0,
                         "generation manifest with extra artifacts must not promote");
            expect(coordinator.getState().status == GenerationCoordinatorStatus::failed,
                   "generation manifest with extra artifacts must fail the coordinator");

            jobDir.deleteRecursively();
        }

        beginTest("cancel in flight prevents concurrent completion promotion");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_cancel_completion_race");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            gateway.blockPoll = true;
            gateway.blockCancel = true;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            const auto runState = coordinator.getState();
            writeValidManifest(jobDir, runState.activeRequestId);

            gateway.pollResponses.push_back(
                {SidecarClientError::none, makeCompletionEvent(runState.activeRequestId)});

            std::thread pollThread([&]() { coordinator.pollOnce(0); });
            while (!gateway.pollEntered)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));

            std::thread cancelThread([&]() { coordinator.cancelActiveGeneration(); });
            while (!gateway.cancelEntered)
                std::this_thread::sleep_for(std::chrono::milliseconds(1));

            gateway.releasePoll = true;
            pollThread.join();

            gateway.releaseCancel = true;
            cancelThread.join();

            expectEquals(history.size(), 0,
                         "completion must not promote while cancellation is in flight");
            expect(coordinator.getState().status != GenerationCoordinatorStatus::succeeded,
                   "race must not leave coordinator succeeded after cancellation");

            jobDir.deleteRecursively();
        }

        // ------------------------------------------------------------------
        // IMPORTANT 3: getState() must be safe to call concurrently while
        // another thread drives pollOnce() progress events.
        beginTest("getState thread safety: concurrent getState and pollOnce do not race");
        {
            const auto jobDir = makeCleanTestDirectory("gc_test_concurrent");

            GeneratedAssetHistory history;
            FakeGateway gateway;
            GenerationCoordinator coordinator{
                history, gateway,
                [&jobDir](const juce::String&) { return jobDir; }};

            coordinator.startGeneration(makeValidForm());
            expect(coordinator.getState().status == GenerationCoordinatorStatus::running,
                   "precondition: should be running");

            // Pre-populate no-message responses so each pollOnce is a no-op.
            constexpr int kPollCount = 100;
            for (int i = 0; i < kPollCount; ++i)
            {
                FakeGateway::PollResponse r;
                r.error = SidecarClientError::none;
                r.event = {};  // empty requestId → ignored by coordinator
                gateway.pollResponses.push_back(r);
            }

            // Background thread calls getState() many times while main thread polls.
            constexpr int kGetStateCount = 500;
            std::thread getStateThread([&]() {
                for (int i = 0; i < kGetStateCount; ++i)
                {
                    auto s = coordinator.getState();
                    (void)s.status;
                }
            });

            for (int i = 0; i < kPollCount; ++i)
                coordinator.pollOnce(0);

            getStateThread.join();

            // Reaching here without crash or sanitizer error means the test passes.
            expect(true, "concurrent getState/pollOnce must complete without data race");

            jobDir.deleteRecursively();
        }
    }
};

static GenerationCoordinatorTests sGenerationCoordinatorTests;

} // namespace acestep_plugin
