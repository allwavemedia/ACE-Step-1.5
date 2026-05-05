#include "AceStepEngine.h"

namespace acestep_plugin
{

namespace
{

bool stubBackendLoader(const juce::File& /*modelsDirectory*/)
{
    return false;
}

GenerationResult stubGenerationRunner(
    const GenerationRequest& /*request*/, const std::atomic<bool>& /*cancel*/)
{
    return { false, {}, "Generation runner not configured." };
}

} // namespace

AceStepEngine::AceStepEngine()
    : backendLoader(stubBackendLoader),
      generationRunner(stubGenerationRunner),
      workerPool(std::make_unique<juce::ThreadPool>(1))
{
}

AceStepEngine::~AceStepEngine()
{
    cancelAll();
    waitForAllJobs();
}

void AceStepEngine::setBackendLoader(BackendLoader loader)
{
    backendLoader = std::move(loader);
}

void AceStepEngine::setGenerationRunner(GenerationRunner runner)
{
    generationRunner = std::move(runner);
}

void AceStepEngine::setProgressCallback(ProgressCallback callback)
{
    progressCallback = std::move(callback);
}

void AceStepEngine::reportProgress(const juce::String& message)
{
    if (progressCallback)
        progressCallback(message);
}

bool AceStepEngine::loadModels(const juce::File& modelsDirectory)
{
    ready = false;
    cancelRequested.store(false, std::memory_order_release);

    if (!backendLoader(modelsDirectory))
        return false;

    ready = true;
    return true;
}

bool AceStepEngine::isReady() const
{
    return ready;
}

GenerationResult AceStepEngine::generate(const GenerationRequest& request)
{
    if (!ready)
        return { false, {}, "Engine is not ready: models have not been loaded." };

    return generationRunner(request, cancelRequested);
}

bool AceStepEngine::submitAsync(const GenerationRequest& request, CompletionCallback onComplete)
{
    if (!ready)
        return false;

    // Reject if a job is already queued or running.
    bool expected = false;
    if (!jobPending.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
        return false;

    cancelRequested.store(false, std::memory_order_release);

    const auto capturedRequest = request;

    workerPool->addJob([this, capturedRequest, cb = std::move(onComplete)] {
        const auto result = generate(capturedRequest);
        jobPending.store(false, std::memory_order_release);

        if (cb)
            cb(result);
    });

    return true;
}

void AceStepEngine::waitForAllJobs()
{
    // Poll until no job is pending; the single worker ensures at most one runs.
    while (jobPending.load(std::memory_order_acquire))
        juce::Thread::sleep(5);
}

void AceStepEngine::cancelAll()
{
    cancelRequested.store(true, std::memory_order_release);
}

} // namespace acestep_plugin
