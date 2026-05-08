#include "SidecarNamedPipeClient.h"

namespace acestep_plugin
{

SidecarNamedPipeClient::SidecarNamedPipeClient(std::unique_ptr<SidecarTransport> transport)
    : _transport(std::move(transport))
{
}

HandshakeResult SidecarNamedPipeClient::performHandshake(
    const juce::String& expectedVersion,
    const juce::StringArray& requiredCapabilities)
{
    if (!_transport || !_transport->isConnected())
        return { SidecarClientError::helperDisconnected, {}, {} };

    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", juce::String("handshake"));
    obj->setProperty("protocolVersion", juce::String(kSidecarProtocolVersion));

    if (!_transport->send(juce::var(obj.get())))
        return { SidecarClientError::helperDisconnected, {}, {} };

    const auto response = _transport->receive(5000);
    if (response.isVoid() || response.isUndefined())
        return { SidecarClientError::helperDisconnected, {}, {} };

    HandshakeResult result;
    result.helperProtocolVersion = response["protocolVersion"].toString();

    if (result.helperProtocolVersion != expectedVersion)
    {
        result.error = SidecarClientError::protocolVersionMismatch;
        return result;
    }

    const auto& caps = response["capabilities"];
    if (caps.isArray())
    {
        for (int i = 0; i < caps.size(); ++i)
            result.capabilities.add(caps[i].toString());
    }

    for (const auto& required : requiredCapabilities)
    {
        if (!result.capabilities.contains(required))
        {
            result.error = SidecarClientError::missingCapability;
            return result;
        }
    }

    return result;
}

SubmitResult SidecarNamedPipeClient::submitJob(const SidecarJobRequest& request)
{
    if (!_transport || !_transport->isConnected())
        return { SidecarClientError::helperDisconnected, {}, {} };

    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", juce::String("submitJob"));
    obj->setProperty("requestId", request.requestId);
    obj->setProperty("sessionId", request.sessionId);
    obj->setProperty("outputDirectory", request.outputDirectory.getFullPathName());

    if (!_transport->send(juce::var(obj.get())))
        return { SidecarClientError::helperDisconnected, {}, {} };

    const auto response = _transport->receive(5000);
    if (response.isVoid() || response.isUndefined())
        return { SidecarClientError::helperDisconnected, {}, {} };

    SubmitResult result;
    result.acknowledgedRequestId = response["requestId"].toString();

    if (result.acknowledgedRequestId != request.requestId)
    {
        result.error = SidecarClientError::requestIdMismatch;
        return result;
    }

    result.artifactRoot = juce::File(response["artifactRoot"].toString());
    return result;
}

SidecarClientError SidecarNamedPipeClient::pollProgress(const juce::String& activeRequestId,
                                                         ProgressEvent& outEvent,
                                                         int timeoutMs)
{
    if (!_transport || !_transport->isConnected())
        return SidecarClientError::helperDisconnected;

    const auto msg = _transport->receive(timeoutMs);
    if (msg.isVoid() || msg.isUndefined())
    {
        outEvent = {};
        return SidecarClientError::none;
    }

    const auto type = msg["type"].toString();
    outEvent.requestId = msg["requestId"].toString();

    if (type == "progress")
    {
        if (outEvent.requestId != activeRequestId)
            return SidecarClientError::staleCompletion;

        outEvent.progressFraction = static_cast<float>(static_cast<double>(msg["progress"]));
        outEvent.statusMessage = msg["status"].toString();
        return SidecarClientError::none;
    }

    if (type == "completion")
    {
        if (outEvent.requestId != activeRequestId)
            return SidecarClientError::staleCompletion;

        return SidecarClientError::none;
    }

    return SidecarClientError::none;
}

SidecarClientError SidecarNamedPipeClient::sendCancellation(const juce::String& requestId)
{
    if (!_transport || !_transport->isConnected())
        return SidecarClientError::helperDisconnected;

    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("type", juce::String("cancel"));
    obj->setProperty("requestId", requestId);

    if (!_transport->send(juce::var(obj.get())))
        return SidecarClientError::helperDisconnected;

    return SidecarClientError::none;
}

bool SidecarNamedPipeClient::isConnected() const
{
    return _transport && _transport->isConnected();
}

} // namespace acestep_plugin
