#pragma once
//
#include <cstdint>
#include <variant>
#include <optional>

//
#include "nlohmann/json.hpp"

#define INVALID_LOCAL_ID std::nullopt
namespace plugin::mqtt
{
    using nlohmann::json;
    struct Timestamp
    {
        Timestamp() = default;
        Timestamp(uint64_t t, double f)
        {
            ticks = t;
            frequency = f;
            seconds = std::nullopt;
        }

        Timestamp(double s)
        {
            ticks = 0;
            frequency = 0;
            seconds = s;
        }

        uint64_t ticks;
        double frequency;
        std::optional<double> seconds;
    };

    using LocalId = std::optional<uint32_t>;

    enum class Datatype
    {
        Number,
        Integer,
        String
    };
    using value_t = std::variant<std::string, int, double>;

    enum class SamplingModes
    {
        Async,
        Sync
    };

    enum class Operation
    {
        Publish,
        Subscribe
    };

    struct Sample
    {
        Sample() = default;
        Sample(const value_t v, Timestamp t)
        {
            time = t;
            push_back(v);
        }

        Sample(std::vector<value_t> v, Timestamp t)
        {
            time = t;
            values.swap(v);
        }

        template <typename T>
        T pop_back()
        {
            auto value = values.back();
            values.pop_back();
            return std::get<T>(value);
        }

        void push_back(value_t value)
        {
            values.push_back(value);
        }

        template <typename T>
        std::vector<T> pop_values()
        {
            std::vector<T> temp;
            std::transform(values.begin(), values.end(), std::back_inserter(temp), [](value_t v) -> T
                           { return std::get<T>(v); });
            values.clear();
            return temp;
        }

        Timestamp time;
        std::vector<value_t> values;
    };
    using Samples = std::vector<Sample>;

    struct Range {
        Range() {
            min = -10;
            max = 10;
            unit = "";
        }

        double min;
        double max;
        std::string unit;
    };

    inline void from_json(const json &j, SamplingModes &s)
    {
        std::string str = j;
        if (str == "async")
        {
            s = SamplingModes::Async;
        }
        else if (str == "sync")
        {
            s = SamplingModes::Sync;
        }
        else
        {
            throw std::invalid_argument("Unknwon sampling-type.");
        }
    }

    inline void from_json(const json &j, Datatype &d)
    {
        std::string str = j;
        if (str == "number")
        {
            d = Datatype::Number;
        }
        else if (str == "integer")
        {
            d = Datatype::Integer;
        }
        else if (str == "string")
        {
            d = Datatype::String;
        }
        else
        {
            throw std::invalid_argument("Unknwon datatype.");
        }
    }
}
