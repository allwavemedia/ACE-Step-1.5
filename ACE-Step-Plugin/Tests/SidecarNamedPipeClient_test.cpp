/** Contract tests for SidecarNamedPipeClient.
 *
 *  All tests use FakeTransport to avoid needing a live named pipe.
 *  Behaviour under test: handshake validation, job submission, progress
 *  correlation, stale-completion detection, and broken-transport handling.
 */
#include "../Source/Sidecar/SidecarNamedPipeClient.h"

#include <juce_core/juce_core.h>

#include <deque>

namespace acestep_plugin
{

// ---------------------------------------------------------------------------
// Test double

/** Fake transport: returns pre-loaded responses from a FIFO queue. */
class FakeTransport final : public SidecarTransport
{
public:
    void enqueue(const juce::var& response) { _responses.push_back(response); }
    void disconnect() { _connected = false; }

    bool send(const juce::var&) override { return _connected; }

    juce::var receive(int /*timeoutMs*/) override
    {
        if (!_connected || _responses.empty())
            return {};
        auto r = _responses.front();
        _responses.pop_front();
        return r;
    }

    bool isConnected() const override { return _connected; }

private:
    std::deque<juce::var> _responses;
    bool _connected = true;
};

// ---------------------------------------------------------------------------
// Message builder helpers

static juce::var makeHandshakeAck(const juce::String& version,
                                   const juce::StringArray& caps)
{
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", juce::String("handshakeAck"));
    obj->setProperty("protocolVersion", version);
    juce::Array<juce::var> arr;
    for (const auto& c : caps)
        arr.add(c);
    obj->setProperty("capabilities", arr);
    return juce::var(obj.get());
}

static juce::var makeJobAck(const juce::String& requestId,
                             const juce::String& artifactRoot)
{
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", juce::String("jobAck"));
    obj->setProperty("requestId", requestId);
    obj->setProperty("artifactRoot", artifactRoot);
    return juce::var(obj.get());
}

static juce::var makeProgressMsg(const juce::String& requestId, float progress)
{
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", juce::String("progress"));
    obj->setProperty("requestId", requestId);
    obj->setProperty("progress", static_cast<double>(progress));
    obj->setProperty("status", juce::String("generating"));
    return juce::var(obj.get());
}

static juce::var makeCompletionMsg(const juce::String& requestId)
{
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", juce::String("completion"));
    obj->setProperty("requestId", requestId);
    return juce::var(obj.get());
}

// ---------------------------------------------------------------------------
// Tests

class SidecarNamedPipeClientTests final : public juce::UnitTest
{
public:
    SidecarNamedPipeClientTests() : juce::UnitTest("SidecarNamedPipeClient") {}

    void runTest() override
    {
        beginTest("handshake accepts matching protocol version and capabilities");
        {
            auto* fake = new FakeTransport();
            fake->enqueue(makeHandshakeAck("1.0", { "generation", "midi" }));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            const auto result = client.performHandshake("1.0", { "generation" });
            expect(result.error == SidecarClientError::none);
            expect(result.helperProtocolVersion == "1.0");
            expect(result.capabilities.contains("generation"));
        }

        beginTest("handshake rejects mismatched protocol version");
        {
            auto* fake = new FakeTransport();
            fake->enqueue(makeHandshakeAck("2.0", { "generation" }));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            const auto result = client.performHandshake("1.0", {});
            expect(result.error == SidecarClientError::protocolVersionMismatch);
        }

        beginTest("handshake rejects missing required capability");
        {
            auto* fake = new FakeTransport();
            fake->enqueue(makeHandshakeAck("1.0", { "generation" }));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            const auto result = client.performHandshake("1.0", { "generation", "stems" });
            expect(result.error == SidecarClientError::missingCapability);
        }

        beginTest("handshake returns helperDisconnected on broken transport");
        {
            auto* fake = new FakeTransport();
            fake->disconnect();
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            const auto result = client.performHandshake("1.0", {});
            expect(result.error == SidecarClientError::helperDisconnected);
        }

        beginTest("submit sends request ID and receives artifact root");
        {
            const juce::String reqId = "test-req-001";
            const juce::String dir = "C:\\jobs\\test-req-001";
            auto* fake = new FakeTransport();
            fake->enqueue(makeJobAck(reqId, dir));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            SidecarJobRequest req;
            req.requestId = reqId;
            req.outputDirectory = juce::File(dir);
            const auto result = client.submitJob(req);

            expect(result.error == SidecarClientError::none);
            expect(result.acknowledgedRequestId == reqId);
            expect(result.artifactRoot.getFullPathName() == dir);
        }

        beginTest("submit detects request ID mismatch in ack");
        {
            auto* fake = new FakeTransport();
            fake->enqueue(makeJobAck("different-id", "C:\\jobs\\x"));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            SidecarJobRequest req;
            req.requestId = "original-id";
            const auto result = client.submitJob(req);

            expect(result.error == SidecarClientError::requestIdMismatch);
        }

        beginTest("progress messages are correlated by request ID");
        {
            const juce::String reqId = "active-req";
            auto* fake = new FakeTransport();
            fake->enqueue(makeProgressMsg(reqId, 0.5f));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            ProgressEvent event;
            const auto err = client.pollProgress(reqId, event, 100);

            expect(err == SidecarClientError::none);
            expect(event.requestId == reqId);
            expect(event.progressFraction > 0.4f && event.progressFraction < 0.6f);
        }

        beginTest("late completion for cancelled request returns staleCompletion");
        {
            const juce::String activeId = "new-req";
            const juce::String cancelledId = "old-req";
            auto* fake = new FakeTransport();
            fake->enqueue(makeCompletionMsg(cancelledId));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            ProgressEvent event;
            const auto err = client.pollProgress(activeId, event, 100);
            expect(err == SidecarClientError::staleCompletion);
        }

        beginTest("broken transport returns helperDisconnected on submit");
        {
            auto* fake = new FakeTransport();
            fake->disconnect();
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            SidecarJobRequest req;
            req.requestId = "req-001";
            const auto result = client.submitJob(req);
            expect(result.error == SidecarClientError::helperDisconnected);
        }

        beginTest("cancellation send returns helperDisconnected on broken transport");
        {
            auto* fake = new FakeTransport();
            fake->disconnect();
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            const auto err = client.sendCancellation("req-001");
            expect(err == SidecarClientError::helperDisconnected);
        }

        beginTest("isConnected reflects transport state");
        {
            auto* fake = new FakeTransport();
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };
            expect(client.isConnected());
            fake->disconnect();
            expect(!client.isConnected());
        }

        // Issue 4: matching completion must set isComplete on ProgressEvent.
        beginTest("matching completion message sets isComplete on ProgressEvent");
        {
            const juce::String reqId = "active-req";
            auto* fake = new FakeTransport();
            fake->enqueue(makeCompletionMsg(reqId));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            ProgressEvent event;
            const auto err = client.pollProgress(reqId, event, 100);

            expect(err == SidecarClientError::none,
                   "matching completion must return none");
            expect(event.isComplete,
                   "isComplete must be true for a matching completion message");
            expect(event.requestId == reqId,
                   "requestId must be set on completion event");
        }

        beginTest("stale completion returns staleCompletion without setting isComplete");
        {
            const juce::String activeId = "new-req";
            const juce::String cancelledId = "old-req";
            auto* fake = new FakeTransport();
            fake->enqueue(makeCompletionMsg(cancelledId));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            ProgressEvent event;
            const auto err = client.pollProgress(activeId, event, 100);
            expect(err == SidecarClientError::staleCompletion,
                   "stale completion must return staleCompletion");
            expect(!event.isComplete,
                   "isComplete must remain false for stale completion");
        }

        beginTest("progress message does not set isComplete");
        {
            const juce::String reqId = "active-req";
            auto* fake = new FakeTransport();
            fake->enqueue(makeProgressMsg(reqId, 0.5f));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            ProgressEvent event;
            const auto err = client.pollProgress(reqId, event, 100);

            expect(err == SidecarClientError::none);
            expect(!event.isComplete,
                   "isComplete must be false for a progress (non-completion) message");
        }

        // Issue 3 (re-review): unknown message type must not leave a partial event.
        beginTest("unknown message type returns unknownMessageType");
        {
            auto* fake = new FakeTransport();
            juce::DynamicObject::Ptr obj = new juce::DynamicObject();
            obj->setProperty("type", juce::String("telemetry"));
            obj->setProperty("requestId", juce::String("active-req"));
            fake->enqueue(juce::var(obj.get()));
            SidecarNamedPipeClient client{ std::unique_ptr<SidecarTransport>(fake) };

            ProgressEvent event;
            const auto err = client.pollProgress("active-req", event, 100);

            expect(err == SidecarClientError::unknownMessageType,
                   "unknown message type must return unknownMessageType");
            expect(event.requestId.isEmpty(),
                   "outEvent.requestId must be cleared for unknown message type");
            expect(!event.isComplete,
                   "outEvent.isComplete must be false for unknown message type");
        }
    }
};

static SidecarNamedPipeClientTests sSidecarNamedPipeClientTests;

} // namespace acestep_plugin
