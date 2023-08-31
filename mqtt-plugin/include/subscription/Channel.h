#pragma once

//
#include "Types.h"
#include "subscription/decoding/Decoder.h"

#include <mqtt/message.h>

//
#include <memory>
#include <vector>

namespace plugin::mqtt
{
    using namespace ::mqtt;
    class Channel
    {
    public:
        struct Configuration
        {
            // The Name of this Channel (e.g. used for default name)
            std::string name;

            // The Unique Identifier of this channel (e.g. for reloading the plugin after changes have been made to the config file)
            std::string uuid;

            // The Datatype processed by this channel
            Datatype datatype;

            // Every Channel has a specific Decoder
            std::shared_ptr<Decoder> decoder;

            // A MQTT-Channel is maped to its OXYGEN Channel by the local channel id
            LocalId local_channel_id;

            // Channel range
            Range range;
        };

        using Pointer = std::shared_ptr<Channel>;

        Channel(Configuration config);

        /**
         * @brief Discard all samples
         */
        void discardSamples();

        /**
         * @brief Get samples and clear buffer
         * @return Samples
         */
        Samples getAndClearSamples();

        /**
         * @brief Get the Local Oxygen Channel Id associated with this MQTT Channel
         * @return LocalId
         */
        LocalId getLocalChannelId();

        /**
         * @brief Interpret payload using channel specific decoder
         * @param Timestamp
         * @param msg
         */
        void interpretPayload(Timestamp start, Timestamp timestamp, const_message_ptr msg);

        /**
         * @brief Get the Datatype of this channel
         * @return Datatype
         */
        Datatype getDatatype();

        /**
         * @brief Get the Configuration of this channel, allows modification
         * @return Configuration&
         */
        Configuration &getConfiguration();

        /**
         * @brief Channels and subscriptions might need to prepare for processing
         */
        void prepareProcessing();

        /**
         * @brief Channels and subscriptions might need to finalize after processing
         */
        void stopProcessing();

        /**
         * @brief Get the Decoder
         * @return std::shared_ptr<Decoder>
         */
        std::shared_ptr<Decoder> getDecoder();

    private:
        Samples m_samples;
        Configuration m_configuration;
    };
}
