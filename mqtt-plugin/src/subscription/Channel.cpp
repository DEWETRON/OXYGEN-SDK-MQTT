#include "subscription/Channel.h"
using namespace plugin::mqtt;

Channel::Channel(Configuration config) : m_configuration(config)
{
}

void Channel::discardSamples()
{
    m_samples.clear();
}

Samples Channel::getAndClearSamples()
{
    Samples temp;
    m_samples.swap(temp);

    return temp;
}

LocalId Channel::getLocalChannelId()
{
    return m_configuration.local_channel_id;
}

void Channel::interpretPayload(Timestamp start, Timestamp timestamp, const_message_ptr msg)
{
    auto sample = m_configuration.decoder->getValue(start, timestamp, msg->to_string());
    m_samples.push_back(sample);
}

Datatype Channel::getDatatype()
{
    return m_configuration.datatype;
}

Channel::Configuration &Channel::getConfiguration()
{
    return m_configuration;
}

std::shared_ptr<Decoder> Channel::getDecoder()
{
    return m_configuration.decoder;
}

void Channel::prepareProcessing()
{
    m_configuration.decoder->prepareProcessing();
}

void Channel::stopProcessing()
{
    m_configuration.decoder->stopProcessing();
}