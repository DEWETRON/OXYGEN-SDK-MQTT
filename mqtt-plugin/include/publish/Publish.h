#pragma once

//
#include "Types.h"

//
#include "odkfw_properties.h"

//
#include "fmt/core.h"
#include "nlohmann/json.hpp"

//
#include <memory>
#include <string>

namespace plugin::mqtt
{
    using nlohmann::json;

    class Publish
    {
    public:
        struct Sampling
        {
            SamplingModes mode;
            int downsampling_factor;
        };

        using Pointer = std::shared_ptr<Publish>;

        /**
         * @brief Construct a new Publish Handler
         * @param topic
         * @param uuid
         * @param sampling
         * @param datatype
         * @param packet_size The packet size if publishing a sync-channel
         * @param QoS
         */
        Publish(std::string topic, std::string uuid, Sampling sampling, Datatype datatype, int packet_size, int QoS);

        /**
         * @brief Get the Uuid
         * @return std::string
         */
        std::string getUuid() const;

        /**
         * @brief Get the Topic
         * @return std::string
         */
        std::string getTopic() const;

        /**
         * @brief Get the QoS
         * @return int
         */
        int getQoS() const;

        /**
         * @brief Get the underlying Oxygen Input channel
         * @return std::shared_ptr<odk::framework::EditableChannelIDProperty>
         */
        std::shared_ptr<odk::framework::EditableChannelIDProperty> getInputChannel() const;

        /**
         * @brief Get the Sampling Settings of this Publish Handler
         * @return Sampling
         */
        Sampling getSampling() const;

        /**
         * @brief Discard all buffers and reset
         */
        void discardSamples();

        /**
         * @brief Add an async sample to be published
         * @tparam T type
         * @param timestamp Oxygen timestamp of current sample
         * @param sample Oxygen sample
         */
        template <typename T>
        void addAsyncSample(double timestamp, T value)
        {
            json j;

            j["sampling"] = "async";
            j["timestamp"] = timestamp;
            j["value"] = value;

            auto str_rep = j.dump();
            m_output_buffer.push_back(str_rep);
        }

        /**
         * @brief Add Sync channels in order (as they are deliverd by Oxygen)
         * @param values
         * @param sample_rate
         */
        void addSyncSamples(std::vector<value_t> values, double sample_rate);

        /**
         * @brief True if there is a payload to publish
         * @return true
         * @return false
         */
        bool hasPayload();

        /**
         * @brief Get payload (FIFO) and remove from buffer
         * @return std::string
         */
        std::string pop();

    private:
        int m_qos;
        Datatype m_datatype;
        std::string m_topic;
        std::string m_uuid;
        Publish::Sampling m_sampling;

        std::shared_ptr<odk::framework::EditableChannelIDProperty> m_input_channel;
        std::vector<value_t> m_input_buffer;
        std::vector<std::string> m_output_buffer;

        // Helpers for sync-channels
        size_t m_next_idx;
        uint64_t m_packet_idx;
        int m_packet_size;
    };
}
