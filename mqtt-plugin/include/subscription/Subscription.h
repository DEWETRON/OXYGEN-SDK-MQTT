#pragma once

//
#include "Types.h"
#include "subscription/Channel.h"
#include "subscription/decoding/Decoder.h"

//
#include <mqtt/message.h>

//
#include <map>
#include <vector>
#include <functional>
#include <string>
#include <optional>

namespace plugin::mqtt
{
    using namespace ::mqtt;

    class Subscription
    {
    public:
        struct Sampling
        {
            SamplingModes mode;
            int timeout;
            std::optional<double> sample_rate;
        };

        using Channels = std::vector<Channel::Pointer>;
        using Pointer = std::shared_ptr<Subscription>;

        Subscription(Sampling sampling, std::string topic, int QoS);

        /**
         * @brief Interpret incoming payloads
         * @param timestamp
         * @param msg
         */
        void interpretPayload(Timestamp start, Timestamp timestamp, const_message_ptr msg);

        /**
         * @brief Discard buffered Samples
         */
        void discardSamples();

        /**
         * @brief Get the Channels of this subscription
         * @return Channels_t
         */
        Channels &getChannels();

        /**
         * @brief Add a channel with a given subscription
         * @param config
         */
        void addChannel(Channel::Pointer channel);

        /**
         * @brief Get the MQTT-Path
         * @return std::string
         */
        std::string &getTopic();

        /**
         * @brief Get the Configuration
         * @return config::Subscription::Pointer
         */
        Sampling &getSampling();

        /**
         * @brief Channels and subscriptions might need to prepare for processing
         */
        void prepareProcessing();

        /**
         * @brief Channels and subscriptions might need to finalize after processing
         */
        void stopProcessing();

        /**
         * @brief Get the Quality of Service for this subscription
         * @return int
         */
        int getQoS();

    private:
        Channels m_channels;
        Sampling m_sampling;
        std::string m_topic;
        int m_qos;
    };
}
