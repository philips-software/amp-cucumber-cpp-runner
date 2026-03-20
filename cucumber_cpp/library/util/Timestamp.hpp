#ifndef UTIL_TIMESTAMP_HPP
#define UTIL_TIMESTAMP_HPP

#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/timestamp.hpp"
#include <chrono>
#include <string>

namespace cucumber_cpp::library::util
{
    constexpr auto millisecondsPerSecond{ 1000u };
    constexpr auto nanosecondsPerMillisecond{ 1000000u };
    constexpr auto nanosecondsPerSecond{ 1000000000u };

    struct TimestampGenerator
    {
    protected:
        TimestampGenerator();
        ~TimestampGenerator();

    public:
        static TimestampGenerator& Instance();
        virtual std::chrono::milliseconds Now() = 0;

    private:
        static inline TimestampGenerator* instance;
    };

    struct TimestampGeneratorSystemClock : TimestampGenerator
    {
        virtual ~TimestampGeneratorSystemClock() = default;

        std::chrono::milliseconds Now() override;
    };

    cucumber::messages::timestamp TimestampNow();

    cucumber::messages::duration operator-(const cucumber::messages::timestamp& lhs, const cucumber::messages::timestamp& rhs);

    std::string MakeIso8601Timestamp(const cucumber::messages::timestamp& timestamp);
}

namespace cucumber::messages
{
    using cucumber_cpp::library::util::operator-;
};

#endif
