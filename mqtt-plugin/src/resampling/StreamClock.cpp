#include "resampling/StreamClock.h"

//
#include <cmath>

using namespace plugin::mqtt;

StreamClock::StreamClock() : m_set(false),
                             m_stream_offset(0),
                             m_base_ticks(0),
                             m_base_frequency(0),
                             m_base_seconds(0)
{
}
void StreamClock::setStartOfStream(double incoming_ts_seconds, std::uint64_t base_ticks, double base_frequency)
{
    m_set = true;
    m_stream_offset = incoming_ts_seconds;
    m_base_ticks = base_ticks;
    m_base_frequency = base_frequency;
    m_base_seconds = base_ticks / base_frequency;
}

void StreamClock::resetSartOfStream()
{
    m_set = false;
    m_stream_offset = 0;
    m_base_ticks = 0;
    m_base_frequency = 0;
    m_base_seconds = 0;
}

bool StreamClock::startOfStreamSet() const
{
    return m_set;
}

std::uint64_t StreamClock::alignSamples(double incoming_ts_seconds, int sample_rate)
{
    const auto sample_interval = 1 / static_cast<double>(sample_rate);
    const auto referenced_stream = incoming_ts_seconds - m_stream_offset;
    const auto stream_ticks = static_cast<std::uint64_t>(referenced_stream / sample_interval);

    const auto rate_factor = sample_rate / static_cast<double>(m_base_frequency);
    std::uint64_t reference_ticks = static_cast<std::uint64_t>(m_base_ticks * rate_factor);

    return stream_ticks + reference_ticks;
}

double StreamClock::alignSeconds(double incoming_ts_seconds)
{
    const auto referenced_stream = incoming_ts_seconds - m_stream_offset;
    const auto aligned_stream = referenced_stream + m_base_seconds;

    return aligned_stream;
}

bool StreamClock::validTimestamp(double incoming_ts_seconds)
{
    if (!m_set)
    {
        return false;
    }

    if (incoming_ts_seconds < m_stream_offset)
    {
        return false;
    }

    return true;
}
