#include "resampling/Stream.h"

//
#include <cmath>
#include <stdexcept>

using namespace plugin::mqtt;

namespace
{
    /**
     * Linear Interpolation function
     */
    inline double lerp(double a, double b, double t)
    {
        return a + (b - a) * t;
    }

    class InputVectorLabels
    {
    public:
        InputVectorLabels(double start, double end, std::size_t size)
            : m_start(start)
              //, m_end(end)
              //, m_size(static_cast<double>(size))
              ,
              m_scale(static_cast<double>(size) / (end - start)) // precompute the index scaling
        {
        }

        inline double indexOfTime(double time) const
        {
            // return m_size * (time - m_start) / (m_end - m_start);
            return m_scale * (time - m_start);
        }

    private:
        double m_start;
        // double m_end;
        // double m_size;
        double m_scale;
    };

    /**
     * computes linearly interpolated output samples and pushes them to the vector
     */
    std::size_t interp(std::vector<double> &output, std::uint64_t x_start, double x_samplerate, std::size_t num, const InputVectorLabels &fp_time, const std::vector<double> &fp)
    {
        for (std::size_t n = 0; n < num; ++n)
        {
            double val = (x_start + n) / x_samplerate;
            double pos = fp_time.indexOfTime(val);

            const int idx = static_cast<int>(std::floor(pos));
            const double t = pos - static_cast<double>(idx);

            // Why do we need to check for negative idx?
            if (idx + 1 >= fp.size() || idx < 0)
            {
                // element fp[idx + 1] is inaccessible, early return
                return n;
            }

            double val_interp = lerp(fp[idx], fp[idx + 1], t);
            output.push_back(val_interp);
        }
        return num;
    }

}

Stream::Stream(StreamClock::Pointer clock, int nominal_sampling_rate) :
    m_clock(clock),
    m_nominal_sampling_rate(nominal_sampling_rate),
    m_nominal_sampling_interval(1 / static_cast<double>(nominal_sampling_rate)),
    m_unrecoverable(false),
    m_actual_scnt(0),
    m_packet_received_counter(0)
{
}

void Stream::reset()
{
    m_unrecoverable = false;
    m_estimated_sampling_rate = std::nullopt;
    m_estimated_sampling_interval = std::nullopt;
    m_output_buffer.clear();
    m_input_buffer.clear();
    m_actual_scnt = 0;
    m_packet_received_counter = 0;

    m_clock->resetSartOfStream();
}

void Stream::ensureValidStreamClock(double incoming_ts_seconds, std::uint64_t base_ticks, double base_frequency)
{
    if (!m_clock->startOfStreamSet())
    {
        // Set the stream clock to align with oxygen time
        m_clock->setStartOfStream(incoming_ts_seconds, base_ticks, base_frequency);
    }

    if (!m_clock->validTimestamp(incoming_ts_seconds))
    {
        throw std::runtime_error("Start of stream (timestamp) not valid, discard packet.");
    }
}

void Stream::beginStream(std::vector<double> &samples, double incoming_ts_seconds)
{
    // Remember nominal packet size
    m_nominal_packet_size = samples.size();

    // Align start of stream with oxygen time
    const auto num_samples = m_clock->alignSamples(incoming_ts_seconds, m_nominal_sampling_rate);
    const int diff = num_samples - samples.size();

    // Reuse as much of the received samples as possible
    if (diff > 0)
    {
        // Append NaN at front
        std::vector<double> NaN;
        NaN.resize(diff);
        std::fill(NaN.begin(), NaN.end(), std::nan(""));

        samples.insert(samples.begin(), NaN.begin(), NaN.end());
    }
    if (diff < 0)
    {
        // Remove some samples at the front
        samples.erase(samples.begin(), samples.begin() + (-1) * diff);
    }

    // Begin sample rate estimation with nominal sample rate
    m_estimated_sampling_rate = m_nominal_sampling_rate;
    m_estimated_sampling_interval = 1 / static_cast<double>(m_estimated_sampling_rate.value());

    // Append to output buffer
    m_output_buffer.insert(m_output_buffer.end(), samples.begin(), samples.end());
    m_actual_scnt += samples.size();
}

void Stream::append(std::vector<double> samples, double incoming_ts_seconds, std::uint64_t base_ticks, double base_frequency)
{
    // Stream still active?
    if (m_unrecoverable)
    {
        throw std::runtime_error("The stream is unrecoverable (too many packets lost?)");
    }

    ensureValidStreamClock(incoming_ts_seconds, base_ticks, base_frequency);
    const auto aligned_ts_seconds = m_clock->alignSeconds(incoming_ts_seconds);
    m_packet_received_counter++;

    if (m_packet_received_counter == 1)
    {
        // Handle start of stream
        beginStream(samples, incoming_ts_seconds);
    }
    else
    {
        // A normal stream packet
        if (samples.size() != m_nominal_packet_size)
        {
            m_unrecoverable = true;
            throw std::runtime_error("Streams are not allowed to change their packet size, unrecoverable.");
        }

        if (aligned_ts_seconds < m_previous_aligned_ts_seconds)
        {
            m_unrecoverable = true;
            throw std::runtime_error("Steady clock expected, unrecoverable.");
        }

        // TODO Find a way to make sure streams are always recoverable (e.g. insert NaN on a regular basis/timeouts?)
        if ((aligned_ts_seconds - m_previous_aligned_ts_seconds) >= 20.0)
        {
            m_unrecoverable = true;
            throw std::runtime_error("Stream lost its integrity for 20 seconds, mark as unrecoverable.");
        }

        // Estimate packet properties
        const auto nominal_packet_interval = m_nominal_packet_size * m_estimated_sampling_interval.value();
        const auto expected_packet_timestamp = m_previous_aligned_ts_seconds + nominal_packet_interval;

        // Map Incoming timestamp to nominal sample rate aligned with Oxygen time
        const auto last_sample_aligned_tick = m_clock->alignSamples(incoming_ts_seconds, m_nominal_sampling_rate);
        const std::size_t num = last_sample_aligned_tick - m_actual_scnt;

        // Based on timestamps, estimate if packets have been lost, if so insert NaN
        if (aligned_ts_seconds > (expected_packet_timestamp + (nominal_packet_interval * 0.25)))
        {
            // Packet lost based on timetamp estimation, align with stream

            if (num < samples.size())
            {
                m_unrecoverable = true;
                throw std::runtime_error("Error while recovering stream.");
            }
            const auto diff = num - samples.size();

            std::vector<double> NaN;
            NaN.resize(static_cast<size_t>(diff));
            std::fill(NaN.begin(), NaN.end(), std::nan(""));

            // Add NaN at front to align stream
            samples.insert(samples.begin(), NaN.begin(), NaN.end());

            // Buffer samples + resampling result
            m_output_buffer.insert(m_output_buffer.end(), samples.begin(), samples.end());

            // Update sample count
            m_actual_scnt += samples.size();
        }
        else
        {
            // Valid packet, resample
            // Re-Estimtae Sampling Rate (only if no packet has been lost)
            m_estimated_sampling_interval = (aligned_ts_seconds - m_previous_aligned_ts_seconds) / m_nominal_packet_size;
            m_estimated_sampling_rate = std::floor(1 / m_estimated_sampling_interval.value());

            // Use previous and current packet to interpolate
            m_input_buffer.insert(m_input_buffer.end(), samples.begin(), samples.end());

            auto estimated_first_sample_timestamp = aligned_ts_seconds - m_input_buffer.size() * m_estimated_sampling_interval.value();
            if (estimated_first_sample_timestamp < 0)
            {
                // TODO is there any better way to overcome/handle this? Estimates before time zero only can occure at start of stream
                estimated_first_sample_timestamp = 0;
            }

            // Prepare Labels for resampling
            InputVectorLabels input_desc(estimated_first_sample_timestamp, aligned_ts_seconds, m_input_buffer.size());

            // write up to <num> interpolated samples to m_output_buffer
            std::vector<double> resampled;
            std::size_t num_written = interp(resampled,
                                             m_actual_scnt, m_nominal_sampling_rate, num, // this iterates over real output timestamps in ticks
                                             input_desc,                                  // Timestamps of input samples
                                             m_input_buffer                               // actual input samples
            );

            m_output_buffer.insert(m_output_buffer.end(), resampled.begin(), resampled.end());
            m_actual_scnt += num_written;
        }
    }

    m_previous_aligned_ts_seconds = aligned_ts_seconds;
    m_input_buffer = samples;
}

std::optional<int> Stream::estimatedSamplingRate()
{
    return m_estimated_sampling_rate;
}

std::vector<double> Stream::getAndClearSamples()
{
    std::vector<double> temp;
    m_output_buffer.swap(temp);

    return temp;
}