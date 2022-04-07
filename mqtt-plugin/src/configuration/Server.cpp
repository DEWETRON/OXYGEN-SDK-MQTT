#include "configuration/Server.h"
using namespace plugin::mqtt;
using namespace plugin::mqtt::config;

std::string Server::getUrl() const
{
    return m_url;
}

void plugin::mqtt::config::from_json(const json &d, Servers &servers)
{
    if (!d.contains("servers"))
    {
        return;
    }

    for (auto server : d["servers"])
    {
        auto config = std::make_shared<Server>();
        config->m_url = server["url"];

        servers.push_back(config);
    }
}
