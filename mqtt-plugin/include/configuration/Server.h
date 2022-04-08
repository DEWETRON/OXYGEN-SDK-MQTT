#pragma once

#include <vector>
#include <memory>
#include <string>

//
#include "nlohmann/json.hpp"

namespace plugin::mqtt::config
{
    using nlohmann::json;
    class Server;
    using Servers = std::vector<std::shared_ptr<Server>>;

    class Server
    {
    public:
        using Pointer = std::shared_ptr<Server>;

        Server() = default;

        /**
         * @brief Get the Url
         * @return std::string
         */
        std::string getUrl() const;

        // Friends
        friend void from_json(const json &d, Servers &t);

    private:
        std::string m_url;
    };

    void from_json(const json &d, Servers &subscriptions);
}
