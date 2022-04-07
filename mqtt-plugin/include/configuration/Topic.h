#pragma once

//
#include "Types.h"
#include "subscription/Channel.h"
#include "subscription/Subscription.h"
#include "publish/Publish.h"
#include "subscription/decoding/Decoder.h"

//
#include <string>
#include <vector>

//
#include "nlohmann/json.hpp"

namespace plugin::mqtt::config
{
    class Topic;
    using nlohmann::json;
    using Topics = std::vector<std::shared_ptr<Topic>>;

    class Topic
    {
    public:
        using Pointer = std::shared_ptr<Topic>;

        /**
         * Every Topic maps to a single subscription. A single Subscription can have multiple MQTT-Channels.
         * Every MQTT-Channel maps to an Oxygen Output Channel. These Output-Channels can be mapped in Groups,
         * depending on the underlying interpreter.
         */
        using Channels = std::vector<Channel::Pointer>;
        struct OxygenOutputChannelMap
        {
            Channels channels;
            std::map<std::string, OxygenOutputChannelMap> group_channels;
        };

        /**
         * @brief Get the Map Containing root-level and grouped channels
         * @return OxygenOutputChannelMap&
         */
        OxygenOutputChannelMap &getOxygenOutputChannelMap();

        /**
         * @brief Get the Subscription object mapped to this topic (configuration)
         * @return Subscription::Pointer
         */
        Subscription::Pointer getSubscription();

        /**
         * @brief Get the Publish Handler
         * @return Publish::Pointer
         */
        Publish::Pointer getPublisher();

        /**
         * @brief Get the Operation (Publish or Subscribe)
         * @return Operation
         */
        Operation getOperation();

        /**
         * @brief Load Topics from JSON-Document
         * This method inserts unique identifiers for each channel into the JSON object
         * @param d
         * @param t
         */
        static void fromJson(json &d, Topics &t);

    private:
        OxygenOutputChannelMap m_output_channel_map;
        Subscription::Pointer m_subscription;
        Publish::Pointer m_publish;
        Operation m_operation;
    };
}