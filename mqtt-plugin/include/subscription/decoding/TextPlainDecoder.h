#pragma once
#include "subscription/decoding/Decoder.h"

namespace plugin::mqtt
{
    class TextPlainDecoder : public Decoder
    {
    public:
        TextPlainDecoder(Datatype d) : Decoder(d) {}

        /**
         * @brief Interpret the given payload (e.g. topic: /my/channel/{payload} where payload is an ASCII decoded number, e.g. 1.25)
         * @param payload
         * @return value_t
         */
        Sample getValue(const Timestamp &start, const Timestamp &timestamp, const std::string &payload) override
        {
            switch (getDatatype())
            {
            case Datatype::Integer:
                return Sample(std::stoi(payload), timestamp);
            case Datatype::Number:
                return Sample(std::stof(payload), timestamp);
            case Datatype::String:
                return Sample(payload, timestamp);
            }

            throw std::runtime_error("We should never get here.");
        }
    };
}
