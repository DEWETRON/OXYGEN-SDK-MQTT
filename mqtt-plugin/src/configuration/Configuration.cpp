#include "configuration/Configuration.h"
#include "configuration/details/Schema.h"

//
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include <fmt/core.h>

//
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

using namespace plugin::mqtt::config;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

bool Configuration::traverse(const json &v, std::list<std::pair<json::json_pointer, json::json_pointer>> &pointers, json::json_pointer &parent)
{
    if (v.is_object())
    {
        // Check if any of the key is "$ref"
        for (auto &[key, item] : v.items())
        {
            if (key == "$ref")
            {
                if (item.is_string())
                {
                    std::string path = item;
                    try
                    {
                        if (path.at(0) == '#')
                        {
                            path.erase(0, 1);
                            pointers.push_back(std::make_pair(parent, json::json_pointer(path)));
                        }
                        else
                        {
                            // TODO: Missing # Token, Only root-level references are supported, inform user
                            return false;
                        }
                    }
                    catch (const std::exception &e)
                    {
                        // TODO: We failed as this pointer is definitly invalid
                        return false;
                    }
                }
            }
        }

        // Iterate next level
        for (auto &[key, item] : v.items())
        {
            parent.push_back(key);
            if (!traverse(item, pointers, parent))
            {
                return false;
            }
            parent.pop_back();
        }
    }

    return true;
}

bool Configuration::dereference(json &d)
{
    std::list<std::pair<json::json_pointer, json::json_pointer>> pointers;
    json::json_pointer parent;

    // Get a list of parent/ref JSON pointers to replace $ref sections with the actual content
    if (!traverse(d, pointers, parent))
    {
        // Getting pointer pairs failed
        return false;
    }

    for (auto [parent_pointer, ref_pointer] : pointers)
    {
        try
        {
            d.at(parent_pointer) = d.at(ref_pointer);
        }
        catch (const std::exception &e)
        {
            // TODO: Swapping failed, show more meaningful errror message?
            return false;
        }
    }
    return true;
}

Configuration::ConfigResult Configuration::loadFileContent(const std::string path)
{
    ConfigResult res;

    // Does File exist?
    namespace fs = std::filesystem;
    if (!fs::exists(path))
    {
        // TODO: Translations?
        res.msg = "File does not exist.";
        res.error = true;
        return res;
    }

    // Load file content
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();

    res.error = false;
    res.cache = buffer.str();
    return res;
}

void Configuration::writeToFile(const std::string path, const json &document)
{
    std::ofstream ofs(path, std::ofstream::trunc);
    ofs << std::setw(4) << document;
    ofs.close();
}

Configuration::ConfigResult Configuration::load(const std::string cache)
{
    ConfigResult res;
    json d;
    try
    {
        d = json::parse(cache);
    }
    catch (json::parse_error &ex)
    {
        res.msg = fmt::format("JSON Parse error at byte {}, given file/cache is not a valid JSON document.", ex.byte);
        res.error = true;

        return res;
    }

    // Load JSON-Schema
    json_validator validator;
    try
    {
        validator.set_root_schema(plugin::mqtt::config::details::configuration_file_schema);
    }
    catch (const std::exception &e)
    {
        // This should never ever happen as the schema is statically build into the plugin.
        res.msg = fmt::format("Validation of JSON schema failed: {}.", e.what());
        res.error = true;
        return res;
    }

    // Dereference JSON
    dereference(d);

    // Check Schema
    try
    {
        // TODO Replace with custom error handler to give show error within
        validator.validate(d);
    }
    catch (const std::exception &e)
    {
        res.msg = fmt::format("Validation of configuration failed: {}.", e.what());
        res.error = true;
        return res;
    }

    // Load subscriptions from JSON
    try
    {
        Topic::fromJson(d, m_topics);
        m_servers = d.get<Servers>();
    }
    catch (const std::exception &e)
    {
        // Actually, this should never happen as we ensure JSON validity by the schema...
        res.msg = "An unknown error occured while parsing the configuration.";
        res.error = true;
        return res;
    }

    res.document = d;
    res.error = false;
    return res;
}

Topics Configuration::getSubscriptions()
{
    Topics subscriptions;
    for (auto topic : m_topics)
    {
        if (topic->getOperation() == Operation::Subscribe)
        {
            subscriptions.push_back(topic);
        }
    }

    return subscriptions;
}

Topics Configuration::getPublishers()
{
    Topics subscriptions;
    for (auto topic : m_topics)
    {
        if (topic->getOperation() == Operation::Publish)
        {
            subscriptions.push_back(topic);
        }
    }

    return subscriptions;
}

Servers Configuration::getServers()
{
    return m_servers;
}