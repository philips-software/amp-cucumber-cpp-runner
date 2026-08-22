#ifndef UTIL_TIMESTAMP_HPP
#define UTIL_TIMESTAMP_HPP

#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/Timestamp.hpp"
#include <chrono>
#include <cstddef>
#include <string>

namespace cucumber_cpp::library::util
{
    constexpr std::size_t millisecondsPerSecond = 1e3;
    constexpr std::size_t nanosecondsPerMillisecond = 1e6;
    constexpr std::size_t nanosecondsPerSecond = 1e9;

    struct TimestampGenerator
    {
        TimestampGenerator();
        virtual ~TimestampGenerator();

        TimestampGenerator(const TimestampGenerator&) = delete;
        TimestampGenerator& operator=(const TimestampGenerator&) = delete;
        TimestampGenerator(TimestampGenerator&&) = delete;
        TimestampGenerator& operator=(TimestampGenerator&&) = delete;

        static TimestampGenerator& Instance();
        static void SetInstance(TimestampGenerator& inst);
        virtual std::chrono::milliseconds Now() = 0;

    private:
        static inline TimestampGenerator* instance;
    };

    struct TimestampGeneratorSystemClock : TimestampGenerator
    {
        virtual ~TimestampGeneratorSystemClock() = default;

        std::chrono::milliseconds Now() override;
    };

    cucumber::messages::Timestamp TimestampNow();

    cucumber::messages::Duration operator-(const cucumber::messages::Timestamp& lhs, const cucumber::messages::Timestamp& rhs);

    std::string MakeIso8601Timestamp(const cucumber::messages::Timestamp& timestamp);
}

namespace cucumber::messages
{
    using cucumber_cpp::library::util::operator-;
};

#endif
