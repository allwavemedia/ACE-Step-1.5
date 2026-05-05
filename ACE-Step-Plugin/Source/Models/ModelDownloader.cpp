#include "ModelDownloader.h"

#include "ModelChecksum.h"

namespace acestep_plugin
{

namespace
{

/** Simple worker that downloads a list of model files sequentially. */
class DownloadWorkerThread final : public juce::Thread
{
public:
    DownloadWorkerThread(
        std::vector<ModelEntry> models,
        juce::File targetDir,
        ModelDownloader::ProgressCallback onProgress,
        ModelDownloader::CompletionCallback onComplete,
        std::atomic<bool>& cancelFlag)
        : juce::Thread("ModelDownloader"),
          models(std::move(models)),
          targetDir(std::move(targetDir)),
          onProgress(std::move(onProgress)),
          onComplete(std::move(onComplete)),
          cancelFlag(cancelFlag)
    {
    }

    void run() override
    {
        const int total = static_cast<int>(models.size());

        for (int i = 0; i < total; ++i)
        {
            if (cancelFlag.load(std::memory_order_acquire) || threadShouldExit())
            {
                invokeCompletion(false, "Download cancelled.");
                return;
            }

            const auto& entry = models[static_cast<std::size_t>(i)];
            const auto destFile = targetDir.getChildFile(entry.filename);

            if (destFile.existsAsFile()
                && (entry.expectedSize == 0 || destFile.getSize() == entry.expectedSize))
            {
                // File already present; skip.
                reportProgress(entry.filename, 1.0f, static_cast<float>(i + 1) / total);
                continue;
            }

            const auto error = downloadFile(entry, destFile, i, total);

            if (!error.isEmpty())
            {
                invokeCompletion(false, error);
                return;
            }
        }

        invokeCompletion(true, {});
    }

private:
    juce::String downloadFile(
        const ModelEntry& entry,
        const juce::File& destFile,
        int fileIndex,
        int totalFiles)
    {
        juce::URL url(entry.downloadUrl);

        // Attempt resume via Range header.
        const auto existingSize = destFile.existsAsFile() ? destFile.getSize() : 0;

        juce::StringPairArray headers;
        if (existingSize > 0)
            headers.set("Range", "bytes=" + juce::String(existingSize) + "-");

        auto stream = url.createInputStream(
            juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withExtraHeaders(headers.getDescription()));

        if (stream == nullptr)
            return "Failed to open URL: " + entry.downloadUrl;

        juce::FileOutputStream out(destFile);
        if (!out.openedOk())
            return "Cannot write to: " + destFile.getFullPathName();

        if (existingSize > 0)
            out.setPosition(existingSize);

        constexpr int bufferSize = 65536;
        juce::HeapBlock<char> buffer(bufferSize);

        juce::int64 written = existingSize;
        const juce::int64 expectedTotal =
            entry.expectedSize > 0 ? entry.expectedSize : -1;

        while (!stream->isExhausted())
        {
            if (cancelFlag.load(std::memory_order_acquire) || threadShouldExit())
                return "Download cancelled.";

            const int read = stream->read(buffer.getData(), bufferSize);
            if (read <= 0)
                break;

            out.write(buffer.getData(), static_cast<std::size_t>(read));
            written += read;

            const float fileProgress =
                (expectedTotal > 0)
                    ? static_cast<float>(written) / static_cast<float>(expectedTotal)
                    : 0.5f;

            const float overall =
                (static_cast<float>(fileIndex) + fileProgress)
                / static_cast<float>(totalFiles);

            reportProgress(entry.filename, fileProgress, overall);
        }

        if (!entry.expectedSha256.isEmpty()
            && !ModelChecksum::verifyFile(destFile, entry.expectedSha256))
        {
            destFile.deleteFile();
            return "Checksum mismatch for " + entry.filename;
        }

        return {};
    }

    void reportProgress(const juce::String& filename, float fileProgress, float overall)
    {
        if (onProgress)
            onProgress(filename, fileProgress, overall);
    }

    void invokeCompletion(bool success, const juce::String& error)
    {
        if (onComplete)
            onComplete(success, error);
    }

    std::vector<ModelEntry> models;
    juce::File targetDir;
    ModelDownloader::ProgressCallback onProgress;
    ModelDownloader::CompletionCallback onComplete;
    std::atomic<bool>& cancelFlag;
};

} // namespace

ModelDownloader::~ModelDownloader()
{
    cancel();
    if (workerThread)
        workerThread->stopThread(5000);
}

void ModelDownloader::start(
    const std::vector<ModelEntry>& models,
    const juce::File& targetDir,
    ProgressCallback onProgress,
    CompletionCallback onComplete)
{
    if (running.load(std::memory_order_acquire))
        return;

    cancelRequested.store(false, std::memory_order_release);
    running.store(true, std::memory_order_release);

    auto wrappedComplete = [this, onComplete](bool success, const juce::String& error) {
        running.store(false, std::memory_order_release);
        if (onComplete)
            onComplete(success, error);
    };

    workerThread = std::make_unique<DownloadWorkerThread>(
        models, targetDir, std::move(onProgress), std::move(wrappedComplete),
        cancelRequested);

    workerThread->startThread();
}

void ModelDownloader::cancel()
{
    cancelRequested.store(true, std::memory_order_release);
}

bool ModelDownloader::isRunning() const noexcept
{
    return running.load(std::memory_order_acquire);
}

} // namespace acestep_plugin
