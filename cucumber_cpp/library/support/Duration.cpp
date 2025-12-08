
#include "cucumber/messages/duration.hpp"
#include "cucumber_cpp/library/support/Duration.hpp"
#include "cucumber_cpp/library/support/Timestamp.hpp"
#include <chrono>
#include <cstddef>

namespace cucumber_cpp::library::support
{
    namespace
    {
        std::chrono::milliseconds ToMillis(std::chrono::seconds seconds, std::chrono::nanoseconds nanos)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(seconds) +
                   std::chrono::duration_cast<std::chrono::milliseconds>(nanos);
        }

        cucumber::messages::duration ToDuration(std::chrono::milliseconds millis)
        {
            return {
                .seconds = millis.count() / millisecondsPerSecond,
                .nanos = (millis.count() % millisecondsPerSecond) * nanosecondsPerMillisecond,
            };
        }
    }

    cucumber::messages::duration MillisecondsToDuration(std::chrono::milliseconds millis)
    {
        return ToDuration(millis);
    }

    std::chrono::milliseconds DurationToMilliseconds(const cucumber::messages::duration& duration)
    {
        return ToMillis(std::chrono::seconds(duration.seconds), std::chrono::nanoseconds(duration.nanos));
    }

    cucumber::messages::duration& operator+=(cucumber::messages::duration& lhs, const cucumber::messages::duration& rhs)
    {
        const auto totalNanos = lhs.nanos + rhs.nanos;
        lhs.seconds += rhs.seconds;
        lhs.seconds += totalNanos / nanosecondsPerSecond;
        lhs.nanos = totalNanos % nanosecondsPerSecond;

        return lhs;
    }
}
