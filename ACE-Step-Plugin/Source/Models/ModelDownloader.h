#pragma once

#include "ModelDiscovery.h"

#include <juce_core/juce_core.h>

#include <atomic>
#include <functional>

namespace acestep_plugin
{

/** Resumable HTTP downloader for ACE-Step model files.
 *
 *  Downloads each missing model file sequentially into the models directory,
 *  supporting HTTP Range requests for resume.  Calls onProgress on a worker
 *  thread; callers must synchronise UI updates via juce::MessageManager.
 */
class ModelDownloader final
{
public:
    /** Called with (filename, perFileProgress 0..1, overallProgress 0..1). */
    using ProgressCallback =
        std::function<void(const juce::String& filename, float fileProgress, float overall)>;

    /** Called when all downloads finish.  success==false means at least one failed. */
    using CompletionCallback = std::function<void(bool success, const juce::String& error)>;

    ModelDownloader() = default;
    ~ModelDownloader();

    /** Start downloading models not already present in targetDir.
     *
     *  No-op if already running; call cancel() and wait before restarting.
     */
    void start(
        const std::vector<ModelEntry>& models,
        const juce::File& targetDir,
        ProgressCallback onProgress,
        CompletionCallback onComplete);

    /** Request cancellation; does not block. */
    void cancel();

    bool isRunning() const noexcept;

private:
    std::atomic<bool> cancelRequested { false };
    std::atomic<bool> running { false };

    std::unique_ptr<juce::Thread> workerThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelDownloader)
};

} // namespace acestep_plugin
