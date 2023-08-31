#include "publish/Publish.h"

using namespace plugin::mqtt;

Publish::Publish(std::string topic, std::string uuid, Publish::Sampling sampling, Datatype datatype, int packet_size, int QoS) :
    m_qos(QoS),
    m_datatype(datatype),
    m_topic(topic),
    m_uuid(uuid),
    m_sampling(sampling),
    m_input_channel(std::make_shared<odk::framework::EditableChannelIDProperty>()),
    m_next_idx(0),
    m_packet_idx(0),
    m_packet_size(packet_size)
{
}

std::string Publish::getUuid() const
{
    return m_uuid;
}

std::string Publish::getTopic() const
{
    return m_topic;
}

std::shared_ptr<odk::framework::EditableChannelIDProperty> Publish::getInputChannel() const
{
    return m_input_channel;
}

Publish::Sampling Publish::getSampling() const
{
    return m_sampling;
}

void Publish::discardSamples()
{
    m_input_buffer.clear();
    m_output_buffer.clear();
    m_packet_idx = 0;
}

int Publish::getQoS() const
{
    return m_qos;
}

void Publish::addSyncSamples(std::vector<value_t> values, double sample_rate)
{
    std::vector<value_t> downsampled;

    // Downsample
    int idx;
    for (idx = m_next_idx; idx < values.size(); idx += m_sampling.downsampling_factor)
    {
        downsampled.push_back(values[idx]);
    }

    // Remember idx to align with next samples
    m_next_idx = idx - values.size();
    m_input_buffer.insert(m_input_buffer.end(), downsampled.begin(), downsampled.end());

    // can we actually create payloads based on currently buffered samples?
    while (m_input_buffer.size() >= m_packet_size)
    {
        // Extract chunck of buffer
        std::vector<value_t> sub(m_input_buffer.begin(), m_input_buffer.begin() + m_packet_size);
        m_input_buffer.erase(m_input_buffer.begin(), m_input_buffer.begin() + m_packet_size);

        // Prepare JSON-Data
        json j;
        j["sampling"] = "sync";
        j["idx"] = m_packet_idx;
        j["data"] = json::array();
        j["sample-rate"] = sample_rate / static_cast<double>(m_sampling.downsampling_factor);
        auto &arr = j["data"];

        try
        {

            for (auto sample : sub)
            {
                switch (m_datatype)
                {
                case Datatype::Integer:
                {
                    const auto out_value = std::get<int>(sample);
                    arr.insert(arr.end(), out_value);
                }
                break;
                case Datatype::Number:
                {
                    const auto out_value = std::get<double>(sample);
                    arr.insert(arr.end(), out_value);
                }
                break;
                }
            }

            m_packet_idx++;

            auto str_rep = j.dump();
            m_output_buffer.push_back(str_rep);
        }
        catch (...)
        {
            // TODO we can end here if we have an issue when extracting a value from std::variant, display error?
        }
    }
}

bool Publish::hasPayload()
{
    return m_output_buffer.size() > 0;
}

std::string Publish::pop()
{
    auto res = m_output_buffer.front();
    m_output_buffer.erase(m_output_buffer.begin());
    return res;
}