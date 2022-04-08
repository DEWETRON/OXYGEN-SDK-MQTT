#pragma once

#include "Types.h"

//
#include <string>
#include <variant>
#include <memory>

namespace plugin::mqtt
{
    /**
     * @brief Decode a single channel from a given payload
     */
    class Decoder
    {
    public:
        Decoder(Datatype d) : m_datatype(d) {}

        /**
         * @brief Get the Value from a given payload
         * @param payload
         * @return value_t
         */
        virtual Sample getValue(const Timestamp &start, const Timestamp &timestamp, const std::string &payload) = 0;

        /**
         * @brief Get the Datatype
         * @return Datatype
         */
        Datatype getDatatype() { return m_datatype; }

        /**
         * @brief Give the decoder a chance to prepare before processing starts
         */
        virtual void prepareProcessing(){};

        /**
         * @brief Decoders might need to finalize operation
         */
        virtual void stopProcessing(){};

    private:
        Datatype m_datatype;
    };
}
