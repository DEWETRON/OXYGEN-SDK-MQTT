#include "subscription/decoding/CborSyncDecoder.h"
using namespace plugin::mqtt;

CborSyncDecoder::CborSyncDecoder(Datatype d, int nominal_sample_rate, StreamClock::Pointer clock) : Decoder(d),
                                                                                                    m_nominal_sample_rate(nominal_sample_rate)
{
    // TODO Make using the global clock for this protocol a config-parameter?
    m_stream = std::shared_ptr<Stream>(new Stream(clock, nominal_sample_rate));
}

void CborSyncDecoder::prepareProcessing()
{
    m_stream->reset();
    m_timestamp = 0;
}

Sample CborSyncDecoder::getValue(const Timestamp &start, const Timestamp &timestamp, const std::string &payload)
{
    auto j = json::from_cbor(payload);
    auto incoming_packet_timestamp = j["timestamp"].get<double>();

    const auto rate_factor = m_nominal_sample_rate / static_cast<double>(timestamp.frequency);
    std::uint64_t target_tick = static_cast<std::uint64_t>(timestamp.ticks * rate_factor);

    std::vector<value_t> samples;
    switch (getDatatype())
    {
    case Datatype::Number:
    {
        auto data = j["data"].get<std::vector<double>>();
        m_stream->append(data, incoming_packet_timestamp, timestamp.ticks, timestamp.frequency);

        data = m_stream->getAndClearSamples();

        std::transform(data.begin(), data.end(), std::back_inserter(samples), [](double v) -> value_t
                       { return v; });
    }
    break;
    default:
        throw std::runtime_error("We should never get here.");
    }

    auto ret = Sample(std::move(samples), Timestamp(m_timestamp, m_nominal_sample_rate));

    m_timestamp += ret.values.size();
    return ret;
}
