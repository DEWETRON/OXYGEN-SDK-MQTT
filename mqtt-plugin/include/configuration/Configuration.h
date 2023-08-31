#pragma once

//
#include <string>
#include <list>

//
#include <nlohmann/json.hpp>

//
#include "Types.h"
#include "configuration/Topic.h"
#include "configuration/Server.h"

namespace plugin::mqtt::config
{
    using nlohmann::json;

    /**
     * @brief The plugin configuration
     */
    class Configuration
    {
    public:
        struct ConfigResult
        {
            std::string cache;
            std::string msg;
            bool error;
            json document;

            ConfigResult()
            {
                cache = "";
                msg = "";
                error = false;
            }
        };

        Configuration() = default;

        /**
         * @brief Simply load content from a file
         * @param path
         * @return ConfigResult
         */
        static ConfigResult loadFileContent(const std::string path);

        /**
         * @brief Write a JSON Document to a file
         * @param path
         * @param document
         */
        static void writeToFile(const std::string path, const json &document);

        /**
         * @brief Load configuration from file
         * @param path
         * @return true
         * @return false
         */
        ConfigResult load(const std::string content);

        /**
         * @brief Get the Subscriptions
         * @return Topics
         */
        Topics getSubscriptions();

        /**
         * @brief Get the Publish Handlers
         * @return Topics
         */
        Topics getPublishers();

        /**
         * @brief Get the Servers
         * @return Servers
         */
        Servers getServers();

    private:
        Topics m_topics;
        Servers m_servers;

        /**
         * @brief Replace all $ref variables with their actual value
         * !!Limited to root-level properties
         * @param d
         * @return true
         * @return false
         */
        bool dereference(json &d);
        bool traverse(const json &v, std::list<std::pair<json::json_pointer, json::json_pointer>> &pointers, json::json_pointer &parent);
    };
}
