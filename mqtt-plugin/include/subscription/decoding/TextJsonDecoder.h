#pragma once
//
#include "subscription/decoding/Decoder.h"

//
#include "nlohmann/json.hpp"

namespace plugin::mqtt
{
    using nlohmann::json;

    class TextJsonDecoder : public Decoder
    {
    public:
        TextJsonDecoder(json::json_pointer schema, Datatype d) : Decoder(d), m_schema(schema) {}

        /**
         * @brief Interpret the given payload (e.g. topic: /my/channel/{payload} where payload is an ASCII decoded json-object, e.g. {"key": 1.25})
         * This implementation uses a JSON-Pointer which is derived from the given config-file (schema-parameter)
         * @param payload
         * @return value_t
         */
        Sample getValue(const Timestamp &start, const Timestamp &timestamp, const std::string &payload) override
        {
            auto j = json::parse(payload);
            switch (getDatatype())
            {
            case Datatype::Integer:
                return Sample(int(j[m_schema]), timestamp);
            case Datatype::Number:
                return Sample(double(j[m_schema]), timestamp);
            case Datatype::String:
                return Sample(std::string(j[m_schema]), timestamp);
            }

            throw std::runtime_error("We should never get here.");
        }

    private:
        json::json_pointer m_schema;
    };
}
