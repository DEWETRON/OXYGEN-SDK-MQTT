#include "subscription/Subscription.h"
using namespace plugin::mqtt;

// Load a test interpreter
#include "subscription/decoding/TextPlainDecoder.h"

Subscription::Subscription(Subscription::Sampling sampling, std::string topic, int QoS) : m_sampling(sampling),
                                                                                          m_topic(topic),
                                                                                          m_qos(QoS)
{
}

void Subscription::interpretPayload(Timestamp start, Timestamp timestamp, const_message_ptr msg)
{
    try
    {
        for (auto &channel : m_channels)
        {
            channel->interpretPayload(start, timestamp, msg);
        }
    }
    catch (const std::exception &e)
    {
        // TODO: Invalid Payload received, show error message?
    }
}

void Subscription::discardSamples()
{
    for (auto &channel : m_channels)
    {
        channel->discardSamples();
    }
}

void Subscription::addChannel(Channel::Pointer channel)
{
    m_channels.push_back(channel);
}

Subscription::Channels &Subscription::getChannels()
{
    return m_channels;
}

std::string &Subscription::getTopic()
{
    return m_topic;
}

Subscription::Sampling &Subscription::getSampling()
{
    return m_sampling;
}

void Subscription::prepareProcessing()
{
    for (auto &channel : m_channels)
    {
        channel->prepareProcessing();
    }
}

void Subscription::stopProcessing()
{
    for (auto &channel : m_channels)
    {
        channel->stopProcessing();
    }
}

int Subscription::getQoS()
{
    return m_qos;
}