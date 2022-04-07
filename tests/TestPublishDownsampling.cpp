#include <catch2/catch_test_macros.hpp>

//
#include "publish/Publish.h"

//
#include "nlohmann/json.hpp"

using namespace plugin::mqtt;
using nlohmann::json;

TEST_CASE("Publish an Oxygen sync-channel with downsampling enabled")
{
    SECTION("Downsample by factor of 2")
    {
        Publish::Sampling sampling;
        sampling.downsampling_factor = 2;
        sampling.mode = SamplingModes::Sync;

        Publish publish("A Topic", "uuid", sampling, Datatype::Integer, 5, 0);
        publish.addSyncSamples({0, 1, 2, 3, 4, 5, 6}, 100);
        publish.addSyncSamples({7, 8, 9, 10}, 100);
        REQUIRE(publish.hasPayload());

        auto payload = publish.pop();
        REQUIRE(publish.hasPayload() == false);
        REQUIRE(payload == "{\"data\":[0,2,4,6,8],\"idx\":0,\"sample-rate\":50.0,\"sampling\":\"sync\"}");
    }
    SECTION("Downsample by factor of 5")
    {
        std::string payload;
        Publish::Sampling sampling;
        sampling.downsampling_factor = 5;
        sampling.mode = SamplingModes::Sync;

        Publish publish("A Topic", "uuid", sampling, Datatype::Integer, 2, 0);
        publish.addSyncSamples({0, 1, 2, 3, 4, 5, 6, 7, 8}, 100);
        publish.addSyncSamples({9, 10, 11, 12, 13, 14, 15, 16, 17}, 100);
        REQUIRE(publish.hasPayload());
        payload = publish.pop();
        REQUIRE(payload == "{\"data\":[0,5],\"idx\":0,\"sample-rate\":20.0,\"sampling\":\"sync\"}");

        REQUIRE(publish.hasPayload());
        payload = publish.pop();
        REQUIRE(publish.hasPayload() == false);
        REQUIRE(payload == "{\"data\":[10,15],\"idx\":1,\"sample-rate\":20.0,\"sampling\":\"sync\"}");
    }
}
