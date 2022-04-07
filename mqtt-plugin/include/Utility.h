#pragma once

//
#include "Types.h"

//
#include "odkfw_properties.h"
#include "odkfw_custom_request_handler.h"
#include "odkfw_software_channel_plugin.h"

/**
 * @brief Simple Helpers to make Oxygen integration easier
 */
inline odk::ChannelDataformat::SampleFormat asOdkFormat(const plugin::mqtt::Datatype datatype)
{
    switch (datatype)
    {
    case plugin::mqtt::Datatype::Integer:
        return odk::ChannelDataformat::SampleFormat::SINT32;
    case plugin::mqtt::Datatype::Number:
        return odk::ChannelDataformat::SampleFormat::DOUBLE;
    case plugin::mqtt::Datatype::String:
        return odk::ChannelDataformat::SampleFormat::UTF8_STRING;
    }

    throw std::invalid_argument("Datatype is unknwon");
}

inline odk::ChannelDataformat::SampleOccurrence asOdkFormat(const plugin::mqtt::SamplingModes sampling)
{
    switch (sampling)
    {
    case plugin::mqtt::SamplingModes::Async:
        return odk::ChannelDataformat::SampleOccurrence::ASYNC;
    case plugin::mqtt::SamplingModes::Sync:
        return odk::ChannelDataformat::SampleOccurrence::SYNC;
    }

    throw std::invalid_argument("Unknwon sample occurence.");
}

std::string getCurrentDllPath();