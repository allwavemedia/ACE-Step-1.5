#pragma once

#include "GenerationRequest.h"
#include "../Stems/StemTypes.h"

#include <juce_core/juce_core.h>

#include <atomic>
#include <functional>
#include <memory>

namespace acestep_plugin
{

/** Manages the ACE-Step inference backend lifetime and single-worker generation.
 *
 *  Generation is dispatched through a single-thread juce::ThreadPool so only
 *  one GGML job runs per plugin instance at any time.  `submitAsync` returns
 *  false if a job is already queued or running.
 *
 *  BackendLoader and GenerationRunner are injectable for testability.
 */
class AceStepEngine final
{
public:
    using BackendLoader = std::function<bool(const juce::File& modelsDirectory)>;

    /** Injectable generation pipeline.  Receives the request and a cancellation
     *  flag; should poll the flag at natural points (e.g. between DiT steps).
     */
    using GenerationRunner =
        std::function<GenerationResult(const GenerationRequest&, const std::atomic<bool>&)>;

    using CompletionCallback = std::function<void(const GenerationResult&)>;
    using ProgressCallback = std::function<void(const juce::String& message)>;

    AceStepEngine();
    ~AceStepEngine();

    void setBackendLoader(BackendLoader loader);
    void setGenerationRunner(GenerationRunner runner);

    /** Set a callback that receives progress messages from the running job.
     *  The callback may be invoked from a worker thread; callers must synchronise.
     */
    void setProgressCallback(ProgressCallback callback);

    /** Report a progress message to the registered callback (if any).
     *  Safe to call from the generation runner inside generate().
     */
    void reportProgress(const juce::String& message);

    /** Report progress for an individual stem group separately from the full mix. */
    void reportStemProgress(StemGroup group, const juce::String& message);

    bool loadModels(const juce::File& modelsDirectory);
    bool isReady() const;

    /** Synchronous generation.  Safe to call from any worker thread. */
    GenerationResult generate(const GenerationRequest& request);

    /** Submit an async generation job.  Returns false if a job is already
     *  running; the submitted callback will not be called in that case.
     */
    bool submitAsync(const GenerationRequest& request, CompletionCallback onComplete);

    /** Block until all pending jobs finish.  Useful in tests and destructors. */
    void waitForAllJobs();

    /** Signal cancellation.  The runner is expected to poll its cancel flag. */
    void cancelAll();

private:
    BackendLoader backendLoader;
    GenerationRunner generationRunner;
    ProgressCallback progressCallback;
    bool ready = false;
    std::atomic<bool> cancelRequested { false };
    std::atomic<bool> jobPending { false };

    std::unique_ptr<juce::ThreadPool> workerPool;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AceStepEngine)
};

} // namespace acestep_plugin
