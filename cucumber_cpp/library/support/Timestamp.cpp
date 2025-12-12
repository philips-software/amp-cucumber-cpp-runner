
#include "cucumber/messages/timestamp.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber_cpp/library/support/Duration.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include <chrono>

namespace cucumber_cpp::library::support
{
    namespace
    {
        std::chrono::milliseconds ToMillis(std::chrono::seconds seconds, std::chrono::nanoseconds nanos)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(seconds) +
                   std::chrono::duration_cast<std::chrono::milliseconds>(nanos);
        }

        std::chrono::milliseconds TimestampToMillis(const cucumber::messages::timestamp& timestamp)
        {
            return ToMillis(std::chrono::seconds(timestamp.seconds), std::chrono::nanoseconds(timestamp.nanos));
        }
    }

    TimestampGenerator::TimestampGenerator()
    {
        instance = this;
    }

    TimestampGenerator::~TimestampGenerator()
    {
        instance = nullptr;
    }

    TimestampGenerator& TimestampGenerator::Instance()
    {
        return *instance;
    }

    std::chrono::milliseconds TimestampGeneratorSystemClock::Now()
    {
        const auto now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now);
    }

    cucumber::messages::timestamp TimestampNow()
    {
        const auto nowMillis = TimestampGenerator::Instance().Now().count();
        const auto seconds = nowMillis / millisecondsPerSecond;
        const auto nanos = (nowMillis % millisecondsPerSecond) * nanosecondsPerMillisecond;
        return cucumber::messages::timestamp{
            .seconds = seconds,
            .nanos = nanos,
        };
    }

    cucumber::messages::duration operator-(const cucumber::messages::timestamp& lhs, const cucumber::messages::timestamp& rhs)
    {
        const auto durationMillis = TimestampToMillis(lhs) - TimestampToMillis(rhs);
        return MillisecondsToDuration(durationMillis);
    }
}
