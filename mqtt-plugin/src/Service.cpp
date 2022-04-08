#include "Service.h"
#include "uuid.h"

using namespace plugin::mqtt;

Service::~Service()
{
    disconnect();
}

void Service::connect()
{
    if (m_client)
        return;

    // Create a random uuid as the connection to the broker will be unique
    auto uuid = uuids::uuid_system_generator{}();
    m_client = std::make_unique<::mqtt::async_client>(m_server_configuration->getUrl(), uuids::to_string(uuid));

    // Let the underlying paho client handle reconnect attempts
    // TODO Make these settings part of the config-file
    m_options.set_automatic_reconnect(true);
    m_options.set_automatic_reconnect(10, 60);

    // The client will always resubscribe to topics of interest when the connection is etasblished
    // Hence, create a clean session
    m_options.set_clean_session(false);

    // Let paho handle MQTT Version handling (including fallbacks)
    m_options.set_mqtt_version(MQTTVERSION_DEFAULT);

    // Install callback and execute connect
    m_client->set_callback(*this);
    m_client->connect(m_options);
}

void Service::disconnect()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (m_client)
    {
        if (m_client->is_connected())
        {
            m_client->disconnect(100)->wait();
            m_client->disable_callbacks();
            m_client.reset();
        }
    }
}

void Service::connected(const std::string &cause)
{
    // TODO: Show Message on connect?

    // Subscribe to all topics
    for (const auto &[path, sub] : m_subscriptions)
    {
        m_client->subscribe(path, sub->getQoS());
    }
}

void Service::connection_lost(const std::string &cause)
{
    // TODO: Show Message when connection is lost
}

void Service::message_arrived(::mqtt::const_message_ptr msg)
{
    std::lock_guard<std::mutex> lock(m_mtx);

    if (!m_timesource)
        return;
    if (!m_enable)
        return;

    // Interpet incoming message
    auto timestamp = m_timesource();

    auto it = m_subscriptions.find(msg->get_topic());
    if (it == m_subscriptions.end())
        return;

    auto subscription = it->second;
    subscription->interpretPayload(m_start, timestamp, msg);
}

void Service::setTimeSource(Timesource timesource)
{
    m_timesource = timesource;
}

void Service::enable()
{
    if (!m_timesource)
        return;

    // Capture Start of Sampling (for sync-channels)
    m_start = m_timesource();
    m_enable = true;
}

void Service::disable()
{
    m_enable = false;

    for (auto subscription : m_subscriptions)
    {
        subscription.second->discardSamples();
    }

    for (auto publish : m_publish_handlers)
    {
        publish.second->discardSamples();
    }
}

Service::Subscriptions Service::getSubscriptions()
{
    Subscriptions subscriptions;

    for (auto it : m_subscriptions)
    {
        subscriptions.push_back(it.second);
    }

    return subscriptions;
}

void Service::addSubscription(Subscription::Pointer sub)
{
    m_subscriptions.insert(std::pair<std::string, Subscription::Pointer>(sub->getTopic(), sub));
}

std::mutex &Service::getLock()
{
    return m_mtx;
}

void Service::prepareProcessing()
{
    std::lock_guard<std::mutex> lock(m_mtx);

    for (auto &[topic, subscription] : m_subscriptions)
    {
        subscription->prepareProcessing();
    }

    enable();
}

void Service::stopProcessing()
{
    std::lock_guard<std::mutex> lock(m_mtx);

    for (auto &[topic, subscription] : m_subscriptions)
    {
        subscription->stopProcessing();
    }

    disable();
}

void Service::setServerConfiguration(config::Server::Pointer config)
{
    m_server_configuration = config;
}

void Service::addPublishHandler(Publish::Pointer pub)
{
    m_publish_handlers.insert(std::pair<std::string, Publish::Pointer>(pub->getTopic(), pub));
}

Service::Publishers Service::getPublishHandlers()
{
    Publishers publishers;

    for (auto it : m_publish_handlers)
    {
        publishers.push_back(it.second);
    }

    return publishers;
}

void Service::publish()
{
    for (auto it : m_publish_handlers)
    {
        auto publisher = it.second;

        while(publisher->hasPayload())
        {
            auto payload = publisher->pop();
            m_client->publish(publisher->getTopic(), payload.data(), payload.size(), publisher->getQoS(), false);
        }
    }
}