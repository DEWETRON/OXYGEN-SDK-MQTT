#pragma once

//
#include <cstdint>
#include <memory>

namespace plugin::mqtt
{
    class StreamClock
    {
    public:
        using Pointer = std::shared_ptr<StreamClock>;
        StreamClock();

        /**
         * @brief Set the Start Of Stream
         * @param incoming_ts_seconds timestamp of last sample in seconds
         * @param base_ticks Oxygen base ticks when first packet of stream arrived
         * @param base_frequency Oxygen base frequency
         */
        void setStartOfStream(double incoming_ts_seconds, std::uint64_t base_ticks, double base_frequency);

        /**
         * @brief Reset start of stream
         */
        void resetSartOfStream();

        /**
         * @brief Has start of stream been set
         * @return true
         * @return false
         */
        bool startOfStreamSet() const;

        /**
         * @brief Align the incoming timestamp in seconds with the Oxygen time in samples since acquistion start
         * @param incoming_ts_seconds timestamp of last sample in seconds
         * @param sample_rate sample rate of stream
         * @return std::uint64_t samples since start of Oxygen time in given sample rate
         */
        std::uint64_t alignSamples(double incoming_ts_seconds, int sample_rate);

        /**
         * @brief Align the incoming timestamp in seconds with the Oxygen time in seconds since acquisition start
         * @param incoming_ts_seconds
         * @param sample_rate
         * @return double seconds since start of Oxygen time
         */
        double alignSeconds(double incoming_ts_seconds);

        /**
         * @brief Test incoming timestamp for sanity
         * @param incoming_ts_seconds
         * @return true
         * @return false if timestamp is negativ (before start of stream), clock has not been set
         */
        bool validTimestamp(double incoming_ts_seconds);

    private:
        bool m_set;
        double m_stream_offset;
        std::uint64_t m_base_ticks;
        double m_base_frequency;
        double m_base_seconds;
    };
}
