#define M_PI 3.141592653589793238463

#include <cmath>
#include <vector>
#include <memory>

//
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

//
#include "resampling/Stream.h"

using namespace plugin::mqtt;
#define BASE_FREQUENCY 1000

class TestStream
{
public:
    struct Sample
    {
        double timestamp;
        double value;

        Sample(double t, double v) : timestamp(t), value(v) {}
    };

    struct Packet
    {
        std::vector<double> samples;
        double timestamp;
    };

    TestStream(int sampling_rate, double SoS, double EoS, int f = 1) : m_sos(SoS),
                                                                       m_sampling_interval(1 / static_cast<double>(sampling_rate)),
                                                                       m_sampling_rate(sampling_rate)
    {
        const auto duration = EoS - SoS;
        const auto num_samples = std::floor(duration / m_sampling_interval);

        for (int i = 0; i < num_samples; i++)
        {
            const auto timestamp = SoS + i * m_sampling_interval;
            const auto sample = std::sin(2 * M_PI * f * (timestamp - SoS));

            m_samples.push_back(Sample(timestamp, sample));
        }
    }

    double startOfStream()
    {
        return m_sos;
    }

    Packet pop(size_t num_samples)
    {
        if (num_samples > m_samples.size())
        {
            throw std::runtime_error("Out of range.");
        }

        Packet p;
        p.timestamp = m_samples[num_samples - 1].timestamp;

        for (int i = 0; i < num_samples; i++)
        {
            Sample s = m_samples.front();
            p.samples.push_back(s.value);
            m_samples.erase(m_samples.begin());
        }

        return p;
    }

    size_t availableSamples()
    {
        return m_samples.size();
    }

private:
    const double m_sos;
    const double m_sampling_interval;
    const double m_sampling_rate;

    std::vector<Sample> m_samples;
};

TEST_CASE("Aligning start of stream")
{
    const auto nominal_sampling_rate = 1000;

    SECTION("Too few samples at start of stream")
    {
        auto clock = std::make_shared<StreamClock>();
        auto handler = Stream(clock, nominal_sampling_rate);
        auto stream = TestStream(nominal_sampling_rate, 0.1, 10);
        auto packet = stream.pop(10);

        handler.append(packet.samples, packet.timestamp, 10, BASE_FREQUENCY);
        auto samples = handler.getAndClearSamples();

        REQUIRE(samples.size() == 10);
    }
    SECTION("Too many samples at start of stream")
    {
        auto clock = std::make_shared<StreamClock>();
        auto handler = Stream(clock, nominal_sampling_rate);
        auto stream = TestStream(nominal_sampling_rate, 0.1, 10);
        auto packet = stream.pop(40);

        handler.append(packet.samples, packet.timestamp, 50, BASE_FREQUENCY);
        auto samples = handler.getAndClearSamples();

        REQUIRE(samples.size() == 50);
    }
    SECTION("A second stream starts after the first stream")
    {
        auto clock = std::make_shared<StreamClock>();
        auto handler_1 = Stream(clock, nominal_sampling_rate);
        auto handler_2 = Stream(clock, nominal_sampling_rate);
        auto stream_1 = TestStream(nominal_sampling_rate, 0.1, 10.1);
        auto stream_2 = TestStream(nominal_sampling_rate, 0.2, 10.2);

        size_t packet_size = stream_1.availableSamples() / 100;
        while (stream_1.availableSamples())
        {
            auto packet = stream_1.pop(packet_size);
            handler_1.append(packet.samples, packet.timestamp, 50, BASE_FREQUENCY);
        }

        packet_size = stream_2.availableSamples() / 100;
        while (stream_2.availableSamples())
        {
            auto packet = stream_2.pop(packet_size);
            handler_2.append(packet.samples, packet.timestamp, 210, BASE_FREQUENCY);
        }

        REQUIRE(stream_1.availableSamples() == 0);
        REQUIRE(stream_2.availableSamples() == 0);

        REQUIRE(handler_1.getAndClearSamples().size() == Catch::Approx(nominal_sampling_rate * 10 - 50).margin(5));
        REQUIRE(handler_2.getAndClearSamples().size() == Catch::Approx(nominal_sampling_rate * 10 + 100 - 50).margin(5));
    }
    SECTION("A second stream starts before first stream, but arrives after (stream-clock already set)")
    {
        auto clock = std::make_shared<StreamClock>();
        auto handler_1 = Stream(clock, nominal_sampling_rate);
        auto handler_2 = Stream(clock, nominal_sampling_rate);
        auto stream_1 = TestStream(nominal_sampling_rate, 0.1, 10);
        auto stream_2 = TestStream(nominal_sampling_rate, 0.08, 10);
        auto packet_1 = stream_1.pop(40);
        auto packet_2 = stream_2.pop(40);

        handler_1.append(packet_1.samples, packet_1.timestamp, 10, BASE_FREQUENCY);
        REQUIRE_THROWS(handler_2.append(packet_2.samples, packet_2.timestamp, 10 + 3, BASE_FREQUENCY));
    }
}

TEST_CASE("Estimating Sampling-Rate.")
{
    const auto nominal_sampling_rate = 1000;
    auto clock = std::make_shared<StreamClock>();
    auto handler = Stream(clock, nominal_sampling_rate);

    REQUIRE(handler.estimatedSamplingRate() == std::nullopt);

    SECTION("Stream sampling rate below nominal sampling rate")
    {
        auto stream = TestStream(989, 0, 10);
        auto packet_1 = stream.pop(100);
        auto packet_2 = stream.pop(100);

        handler.append(packet_1.samples, packet_1.timestamp, 0, BASE_FREQUENCY);
        handler.append(packet_2.samples, packet_2.timestamp, 100, BASE_FREQUENCY);
        REQUIRE(handler.estimatedSamplingRate().value() == 989);
    }
    SECTION("Stream sampling rate higher than nominal sampling rate")
    {
        auto stream = TestStream(1102, 0, 10);
        auto packet_1 = stream.pop(100);
        auto packet_2 = stream.pop(100);

        handler.append(packet_1.samples, packet_1.timestamp, 0, BASE_FREQUENCY);
        handler.append(packet_2.samples, packet_2.timestamp, 100, BASE_FREQUENCY);
        REQUIRE(handler.estimatedSamplingRate().value() == 1102);
    }
}

TEST_CASE("Resampling Stream")
{
    const auto nominal_sampling_rate = 1000;
    auto clock = std::make_shared<StreamClock>();
    auto handler = Stream(clock, nominal_sampling_rate);

    REQUIRE(handler.estimatedSamplingRate() == std::nullopt);

    SECTION("Sampling rate of incoming stream is below nominal sampling rate.")
    {
        int idx = 1;
        auto stream = TestStream(990, 0, 10);
        const auto packet_size = stream.availableSamples() / 10;

        REQUIRE(stream.availableSamples() == 990 * 10);
        while (stream.availableSamples())
        {
            auto packet = stream.pop(packet_size);
            handler.append(packet.samples, packet.timestamp, 1000 * idx, BASE_FREQUENCY);
            idx++;
        }

        auto samples = handler.getAndClearSamples();
        REQUIRE(handler.estimatedSamplingRate() == 990);
        REQUIRE(samples.size() == Catch::Approx(1000 * 10).margin(5));
    }
    SECTION("Sampling rate is above incoming stream.")
    {
        int idx = 1;
        auto stream = TestStream(1049, 0, 10);
        const auto packet_size = stream.availableSamples() / 10;

        REQUIRE(stream.availableSamples() == 1049 * 10);
        while (stream.availableSamples())
        {
            auto packet = stream.pop(packet_size);
            handler.append(packet.samples, packet.timestamp, 1000 * idx, BASE_FREQUENCY);
            idx++;
        }

        auto samples = handler.getAndClearSamples();
        REQUIRE(handler.estimatedSamplingRate() == 1049);
        REQUIRE(samples.size() == Catch::Approx(1000 * 10).margin(5));
    }
}

TEST_CASE("Recover from a broken stream")
{
    const auto nominal_sampling_rate = 1000;
    auto clock = std::make_shared<StreamClock>();
    auto handler = Stream(clock, nominal_sampling_rate);

    SECTION("Stream looses integrity and restarts after one packet has been lost")
    {
        const int packet_size = 100;
        int idx = 1;

        auto stream_part_1 = TestStream(990, 0, 10);
        auto stream_part_2 = TestStream(1000, 10.1, 20);

        while (stream_part_1.availableSamples())
        {
            auto packet = stream_part_1.pop(packet_size);
            handler.append(packet.samples, packet.timestamp, packet_size * idx, BASE_FREQUENCY);
            idx++;
        }
        REQUIRE(handler.estimatedSamplingRate().value() == Catch::Approx(990).margin(5));

        while (stream_part_2.availableSamples())
        {
            auto packet = stream_part_2.pop(packet_size);
            handler.append(packet.samples, packet.timestamp, packet_size * idx, BASE_FREQUENCY);
            idx++;
        }

        auto samples = handler.getAndClearSamples();
        REQUIRE(handler.estimatedSamplingRate().value() == Catch::Approx(1000).margin(5));
        REQUIRE(samples.size() == Catch::Approx(1000 * 20).margin(5));
    }
    SECTION("Stream looses integrity and restarts after at least five packets have been lost")
    {
        const int packet_size = 100;
        int idx = 1;

        auto stream_part_1 = TestStream(990, 0, 10);
        auto stream_part_2 = TestStream(1000, 10.6, 20);

        while (stream_part_1.availableSamples())
        {
            auto packet = stream_part_1.pop(packet_size);
            handler.append(packet.samples, packet.timestamp, packet_size * idx, BASE_FREQUENCY);
            idx++;
        }
        REQUIRE(handler.estimatedSamplingRate().value() == Catch::Approx(990).margin(5));

        while (stream_part_2.availableSamples())
        {
            auto packet = stream_part_2.pop(packet_size);
            handler.append(packet.samples, packet.timestamp, packet_size * idx, 1000);
            idx++;
        }

        auto samples = handler.getAndClearSamples();
        REQUIRE(handler.estimatedSamplingRate().value() == Catch::Approx(1000).margin(5));

        // TODO Reduce margin?
        REQUIRE(samples.size() == Catch::Approx(1000 * 20).margin(5));
    }
    SECTION("Stream is unrecoverable")
    {
        const int packet_size = 100;
        int idx = 1;

        auto stream_part_1 = TestStream(990, 0, 10);
        auto stream_part_2 = TestStream(1000, 40, 50);

        while (stream_part_1.availableSamples())
        {
            auto packet = stream_part_1.pop(packet_size);
            handler.append(packet.samples, packet.timestamp, packet_size * idx, BASE_FREQUENCY);
            idx++;
        }
        REQUIRE(handler.estimatedSamplingRate().value() == Catch::Approx(990).margin(5));

        while (stream_part_2.availableSamples())
        {
            auto packet = stream_part_2.pop(packet_size);
            REQUIRE_THROWS(handler.append(packet.samples, packet.timestamp, packet_size * idx, BASE_FREQUENCY));
            idx++;
        }
    }
}

TEST_CASE("Test boundaries")
{
    SECTION("The difference between nominal and actual sampling rate is too high.")
    {
        // TODO Implement
    }
}
