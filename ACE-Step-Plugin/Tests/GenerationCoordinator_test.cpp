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

#include <deque>
#include <optional>

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
    std::deque<PollResponse> pollResponses;
    juce::String lastCancelledRequestId;
    std::optional<SidecarJobRequest> lastSubmittedRequest;

    SubmitResult submitGeneration(const SidecarJobRequest& request) override
    {
        lastSubmittedRequest = request;
        SubmitResult result;
        result.error = submitError;
        if (submitError == SidecarClientError::none)
        {
            result.acknowledgedRequestId = request.requestId;
            result.artifactRoot = request.outputDirectory;
        }
        return result;
    }

    SidecarClientError pollProgress(const juce::String& /*requestId*/,
                                    ProgressEvent& outEvent,
                                    int /*timeoutMs*/) override
    {
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
        return SidecarClientError::none;
    }
};

// ---------------------------------------------------------------------------
// Helpers

/** Write a valid manifest + artifact file in jobDir for requestId. */
static void writeValidManifest(const juce::File& jobDir,
                                const juce::String& requestId,
                                const juce::String& artifactName = "full_mix.wav")
{
    const auto artFile = jobDir.getChildFile(artifactName);
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
            const auto jobDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("gc_test_success");
            jobDir.createDirectory();

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
        }

        // ------------------------------------------------------------------
        beginTest("completion with corrupt manifest does not add history");
        {
            const auto jobDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("gc_test_badmanifest");
            jobDir.createDirectory();

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
            const auto jobDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("gc_test_nomanifest");
            jobDir.createDirectory();

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
            const auto jobDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("gc_test_badreqid");
            jobDir.createDirectory();

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
            const auto jobDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("gc_test_cancel");
            jobDir.createDirectory();

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
            const auto jobDir1 = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                     .getChildFile("gc_test_preserve1");
            const auto jobDir2 = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                     .getChildFile("gc_test_preserve2");
            jobDir1.createDirectory();
            jobDir2.createDirectory();

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
            const auto jobDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("gc_test_params");
            jobDir.createDirectory();

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
            const auto jobDir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                    .getChildFile("gc_test_already");
            jobDir.createDirectory();

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
    }
};

static GenerationCoordinatorTests sGenerationCoordinatorTests;

} // namespace acestep_plugin
