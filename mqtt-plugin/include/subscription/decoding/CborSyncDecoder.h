#pragma once
#include "subscription/decoding/Decoder.h"
#include "resampling/Stream.h"
#include "resampling/StreamClock.h"

//
#include "nlohmann/json.hpp"

//
#include <memory>

namespace plugin::mqtt
{
    class CborSyncDecoder : public Decoder
    {
    public:
        CborSyncDecoder(Datatype d, int nominal_sample_rate, StreamClock::Pointer clock);
        void prepareProcessing() override;

        /**
         * @brief Extract values from payload. This is a sync decoder implementing resampling!
         * @param start
         * @param timestamp
         * @param payload
         * @return Sample
         */
        Sample getValue(const Timestamp &start, const Timestamp &timestamp, const std::string &payload) override;

    private:
        int m_nominal_sample_rate;
        std::uint64_t m_timestamp;
        std::shared_ptr<Stream> m_stream;
    };
}
