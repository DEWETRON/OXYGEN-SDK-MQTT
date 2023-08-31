#pragma once

//
#include "configuration/Server.h"
#include "subscription/Subscription.h"
#include "publish/Publish.h"
#include "Types.h"

//
#include <fmt/core.h>
#include <mqtt/async_client.h>

//
#include <memory>
#include <functional>
#include <mutex>

namespace plugin::mqtt
{
    using namespace ::mqtt;

    /**
     * @brief Providing MQTT Service to Publishers and Subscribers
     */
    class Service : public virtual callback
    {
    public:
        using Timesource = std::function<Timestamp(void)>;
        using Subscriptions = std::vector<Subscription::Pointer>;
        using Publishers = std::vector<Publish::Pointer>;
        Service() = default;
        ~Service();

        /**
         * @brief Establish a connection to the broker based on the given configuration
         */
        void connect();

        /**
         * @brief Disconnect and clear all states
         */
        void disconnect();

        /**
         * @brief Set the Time Source for the Subscriptions. Will get called on each incoming message
         * @param currentTick
         */
        void setTimeSource(Timesource timesource);

        /**
         * @brief Get all subscriptions
         * @return std::vector<Subscription>
         */
        Subscriptions getSubscriptions();

        /**
         * @brief Get the Publishers handled by this service
         * @return Publishers
         */
        Publishers getPublishHandlers();

        /**
         * @brief Add a subscription to the service
         * @param sub
         */
        void addSubscription(Subscription::Pointer sub);

        /**
         * @brief Add a publish-handler to the service
         * @param pub
         */
        void addPublishHandler(Publish::Pointer pub);

        /**
         * @brief Get access to the internal lock to prevent changes in internal buffers
         * @return std::mutex&
         */
        std::mutex &getLock();

        /**
         * @brief Prepare the service and its Channels for processing
         */
        void prepareProcessing();

        /**
         * @brief Give the service and its Channels a chance to finalize processing
         */
        void stopProcessing();

        /**
         * @brief Set the Server Configuration
         * @param config
         */
        void setServerConfiguration(config::Server::Pointer config);

        /**
         * @brief Iterate over all publish handlers and send out payloads if any
         */
        void publish();

    private:
        /**
         * @brief Enable sampling
         */
        void enable();

        /**
         * @brief Disable sampling and discard all buffered samples
         */
        void disable();

        /**
         * @brief (Re)connecting success callback
         * @param tok
         */
        void connected(const std::string &cause) override;

        /**
         * @brief Callback indicatiSubscriptionng connection-loss. An automatic reconnect will be triggered
         * @param cause
         */
        void connection_lost(const std::string &cause) override;

        /**
         * @brief Callback indicating async message arived
         * @param msg
         */
        void message_arrived(const_message_ptr msg) override;

        std::unique_ptr<async_client> m_client;
        config::Server::Pointer m_server_configuration;
        connect_options m_options;
        Timesource m_timesource;
        std::mutex m_mtx;
        bool m_enable;
        Timestamp m_start;

        // Map Subscription Object to Topics
        std::map<std::string, Subscription::Pointer> m_subscriptions;
        std::map<std::string, Publish::Pointer> m_publish_handlers;
    };
}
