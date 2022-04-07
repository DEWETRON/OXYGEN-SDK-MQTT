#include "configuration/Topic.h"
#include "subscription/decoding/TextJsonDecoder.h"
#include "subscription/decoding/TextPlainDecoder.h"
#include "subscription/decoding/CborSyncDecoder.h"
#include "resampling/StreamClock.h"

//
#include "fmt/core.h"
#include "uuid.h"

using namespace plugin::mqtt;
using namespace plugin::mqtt::config;

Topic::OxygenOutputChannelMap &Topic::getOxygenOutputChannelMap()
{
    return m_output_channel_map;
}

Subscription::Pointer Topic::getSubscription()
{
    return m_subscription;
}

Publish::Pointer Topic::getPublisher()
{
    return m_publish;
}

Operation Topic::getOperation()
{
    return m_operation;
}

namespace
{
    inline void traverseJsonSchemaChannels(json &j, Topic::OxygenOutputChannelMap &map, json::json_pointer &pointer, Subscription::Pointer subscription)
    {
        for (auto &[key, value] : j.items())
        {
            if (value["type"] != "object")
            {
                // The datatype of this channel
                auto datatype = value["type"].get<Datatype>();

                std::string uuid;
                if (value.count("__uuid") > 0)
                {
                    uuid = value["__uuid"].get<std::string>();
                }
                else
                {
                    auto generator = uuids::uuid_system_generator{}();
                    uuid = uuids::to_string(generator);
                    value["__uuid"] = uuid;
                }

                // Channel Range
                Range range;
                if (value.contains("range"))
                {
                    range.min = value["range"]["min"].get<double>();
                    range.max = value["range"]["max"].get<double>();

                    if (value["range"].contains("unit"))
                    {
                        range.unit = value["range"]["unit"].get<std::string>();
                    }
                }

                // Create a channel and its interpreter
                Channel::Configuration configuration;
                configuration.name = key;
                configuration.uuid = uuid;
                configuration.datatype = datatype;
                configuration.range = range;
                configuration.local_channel_id = INVALID_LOCAL_ID;

                // Append the key to the json-path
                pointer.push_back(key);

                // Create Decoder
                configuration.decoder = std::make_shared<TextJsonDecoder>(pointer, datatype);

                // Reset JSON-Pointer
                pointer.pop_back();

                // Create channel and add to subscription
                auto channel = std::make_shared<Channel>(std::move(configuration));
                subscription->addChannel(channel);

                // add to channels of current group
                map.channels.push_back(channel);
            }
            else if (value["type"] == "object")
            {
                // The current key defines the new group and path in the incoming JSON payload as well as a subgroup for the oxygen-channels
                auto &sub_map = map.group_channels[key];
                pointer.push_back(key);

                traverseJsonSchemaChannels(value["properties"], sub_map, pointer, subscription);

                // Remove instances
                pointer.pop_back();
            }
        }
    }

    inline void loadOutputChannelsFromJsonSchema(const std::string &path, json &j, Topic::OxygenOutputChannelMap &root, Subscription::Pointer subscription)
    {
        // All Schema-Channels are mapped to the path of this topic
        auto &group = root.group_channels[path];

        // The JSON-Pointer is relative to the schema object and will be used by the decoder
        json::json_pointer pointer("");

        // Walk/traverse through the schema, create all channels and add them to the subscription as well as the Oxygen Output Channel Map
        traverseJsonSchemaChannels(j, group, pointer, subscription);
    }

    inline std::string insertOrGetUuidFromSchema(json &schema)
    {
        std::string uuid;
        if (schema.contains("__uuid"))
        {
            uuid = schema.at("__uuid").get<std::string>();
        }
        else
        {
            auto generator = uuids::uuid_system_generator{}();
            uuid = uuids::to_string(generator);
            schema["__uuid"] = uuid;
        }

        return uuid;
    }
}

void Topic::fromJson(json &d, Topics &topics)
{
    if (!d.contains("topics"))
    {
        return;
    }

    std::map<std::string, StreamClock::Pointer> stream_clocks;

    // Load Topics, filter for subscriptions
    for (auto &[path, item] : d["topics"].items())
    {
        // MQTT Protocol Settings
        int QoS = 0;
        if (item.contains("QoS"))
        {
            QoS = item["QoS"].get<int>();
        }

        if (item.contains("subscribe"))
        {
            auto topic = std::make_shared<Topic>();
            topic->m_operation = Operation::Subscribe;

            // Sampling
            Subscription::Sampling sampling;
            sampling.mode = item["/subscribe/sampling/type"_json_pointer].get<SamplingModes>();

            std::string clock_domain = "";
            if (item["/subscribe/sampling"_json_pointer].contains("clock"))
            {
                clock_domain = item["/subscribe/sampling/clock"_json_pointer].get<std::string>();
            }

            if (sampling.mode == SamplingModes::Sync)
            {
                sampling.sample_rate = item["/subscribe/sampling/sample-rate"_json_pointer].get<double>();
            }

            // The underlying Subscription object
            auto subscription = std::make_shared<Subscription>(std::move(sampling), path, QoS);
            topic->m_subscription = subscription;

            auto &payload = item["/subscribe/payload"_json_pointer];

            // Payload - Different Interpreters
            if (payload.contains("text/plain"))
            {
                auto &schema = payload["/text~1plain/schema"_json_pointer];
                // The Unique-Identifier of this channel (get or create)
                auto uuid = insertOrGetUuidFromSchema(schema);
                // The Datatype of this channel
                auto datatype = schema["type"].get<Datatype>();
                // The range of the channel
                Range range;
                if (schema.contains("range"))
                {
                    range.min = schema["range"]["min"].get<double>();
                    range.max = schema["range"]["max"].get<double>();

                    if (schema["range"].contains("unit"))
                    {
                        range.unit = schema["range"]["unit"].get<std::string>();
                    }
                }

                Channel::Configuration configuration;
                configuration.name = path;
                configuration.uuid = uuid;
                configuration.datatype = datatype;
                configuration.decoder = std::make_shared<TextPlainDecoder>(datatype);
                configuration.range = range;
                configuration.local_channel_id = INVALID_LOCAL_ID;

                // Create a channel and add it to the subscription
                auto channel = std::make_shared<Channel>(std::move(configuration));
                subscription->addChannel(channel);

                // Append Channel to the Oxygen Output Channel Map as a Root-Level Channel
                topic->m_output_channel_map.channels.push_back(channel);
            }
            else if (payload.contains("text/json"))
            {
                // Load all Channels from the Configuration-Schema
                auto &schema = payload["/text~1json/schema"_json_pointer];
                loadOutputChannelsFromJsonSchema(path, schema, topic->m_output_channel_map, subscription);
            }
            else if (payload.contains("cbor/json/sync"))
            {
                auto &schema = payload["/cbor~1json~1sync/schema"_json_pointer];
                // The Unique-Identifier of this channel (get or create)
                auto uuid = insertOrGetUuidFromSchema(schema);
                // The Datatype of this channel
                auto datatype = schema["type"].get<Datatype>();

                // Create or get the Stream-Clock for this subscription
                StreamClock::Pointer clock;

                if (clock_domain.empty())
                {
                    // The stream does not share a clock domain
                    clock = std::make_shared<StreamClock>();
                }
                else
                {
                    if (stream_clocks.count(clock_domain))
                    {
                        clock = stream_clocks[clock_domain];
                    }
                    else
                    {
                        clock = std::make_shared<StreamClock>();
                        stream_clocks[clock_domain] = clock;
                    }
                }

                // Channel Range
                Range range;
                if (schema.contains("range"))
                {
                    range.min = schema["range"]["min"].get<double>();
                    range.max = schema["range"]["max"].get<double>();

                    if (schema["range"].contains("unit"))
                    {
                        range.unit = schema["range"]["unit"].get<std::string>();
                    }
                }

                Channel::Configuration configuration;
                configuration.name = path;
                configuration.uuid = uuid;
                configuration.datatype = datatype;
                configuration.decoder = std::make_shared<CborSyncDecoder>(datatype, sampling.sample_rate.value(), clock);
                configuration.range = range;
                configuration.local_channel_id = INVALID_LOCAL_ID;

                // Create a channel and add it to the subscription
                auto channel = std::make_shared<Channel>(std::move(configuration));
                subscription->addChannel(channel);

                // Cbor-Sync requires sampling to be of mode sync!
                if (sampling.mode != SamplingModes::Sync)
                {
                    throw std::invalid_argument(fmt::format("Sampling mode of {} must be of type sync when using cbor/json/sync payload decoder.", path));
                }

                // Append Channel to the Oxygen Output Channel Map as a Root-Level Channel
                topic->m_output_channel_map.channels.push_back(channel);
            }

            // Finally append to topics
            topics.push_back(std::move(topic));
        }
        else if (item.contains("publish"))
        {
            auto topic = std::make_shared<Topic>();
            topic->m_operation = Operation::Publish;

            // Sampling
            Publish::Sampling sampling;
            auto &s = item["/publish/sampling"_json_pointer];
            sampling.mode = s["type"].get<SamplingModes>();
            sampling.downsampling_factor = 1;

            if (s.contains("downsampling-factor"))
            {
                sampling.downsampling_factor = s["downsampling-factor"].get<int>();
            }

            auto &p = item["/publish/payload"_json_pointer];
            auto datatype = p["type"].get<Datatype>();
            int packet_size = 10;

            if (p.contains("samples-per-packet"))
            {
                packet_size = p["samples-per-packet"].get<int>();
            }

            // Oxygen Channel ID and UUID
            std::string uuid = "";
            std::string oxygen_channel = "";

            if (item.contains("__channel"))
            {
                auto &c = item["__channel"];

                if (c.contains("__uuid"))
                {
                    uuid = c["__uuid"].get<std::string>();
                }
                else
                {
                    auto generator = uuids::uuid_system_generator{}();
                    uuid = uuids::to_string(generator);

                    c["__uuid"] = uuid;
                }
            }
            else
            {
                auto generator = uuids::uuid_system_generator{}();
                uuid = uuids::to_string(generator);

                item["__channel"]["__uuid"] = uuid;
            }

            auto publish = std::make_shared<Publish>(path, uuid, sampling, datatype, packet_size, QoS);
            topic->m_publish = publish;

            topics.push_back(std::move(topic));
        }
    }
}
