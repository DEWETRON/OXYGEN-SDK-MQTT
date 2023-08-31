#pragma once

#include "resampling/StreamClock.h"

//
#include <cstdint>
#include <optional>
#include <vector>

namespace plugin::mqtt
{
    class Stream
    {
    public:
        Stream(StreamClock::Pointer clock, int nominal_sampling_rate);

        /**
         * @brief Reset the stream handler and its resampler
         */
        void reset();

        /**
         * @brief Append and resample incoming samples to the local buffer
         *
         * The stream estimates the streaming rate of the incoming stream and resamples it to match
         * the nomial sample rate. The stream class also handles aligning the start of stream to the Oxygen time.
         *
         * The stream class inserts NaN samples at the beginning of a stream if packets are missing to align
         * the incoming timestamp with the stream clock (which can be global to sync multiple stream sources).
         *
         * If packets are lost (stream looses integrity), the handler will try to recover the stream
         * by inserting the missing amount of samples as NaN
         *
         * The underlying network protocol is responsible to ensure that packets are delivered exactly once
         * and that the packet transmission is kept in the correct order (e.g. MQTT QoS 2). The Resampler
         * can tolerate a drift of +/- 10% of nominal sample rate.
         *
         * @param samples
         * @param incoming_ts timestamp of the last sample in seconds (should be part of the protocol)
         * @param local_tick the current oxygen target tick (respecting the correct sampling rate!)
         */
        void append(std::vector<double> samples, double incoming_ts_seconds, std::uint64_t base_ticks, double base_frequency);

        /**
         * @brief Get the estimated sampling rate once the stream has received sufficient data
         * @return std::optional<int>
         */
        std::optional<int> estimatedSamplingRate();

        /**
         * @brief Get the And Clear buffered (resampled) samples aligned with Oxygen Stream
         * @return std::vector<double>
         */
        std::vector<double> getAndClearSamples();

    private:
        /**
         * @brief Make sure stream clock has been set with first packet of stream arriving
         */
        void ensureValidStreamClock(double incoming_ts_seconds, std::uint64_t base_ticks, double base_frequency);

        /**
         * @brief Begin a stream (manage first packet)
         * @param samples The samples of the packet
         * @param incoming_ts_seconds The timestamp of the last sample in the packet
         */
        void beginStream(std::vector<double> &samples, double incoming_ts_seconds);

        StreamClock::Pointer m_clock;

        const int m_nominal_sampling_rate;
        const double m_nominal_sampling_interval;
        bool m_unrecoverable;

        std::optional<double> m_estimated_sampling_interval;
        std::optional<int> m_estimated_sampling_rate;
        size_t m_nominal_packet_size;
        std::uint64_t m_actual_scnt;
        std::uint64_t m_packet_received_counter;
        double m_previous_aligned_ts_seconds;

        std::vector<double> m_output_buffer;
        std::vector<double> m_input_buffer;
    };
}