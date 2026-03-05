
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include <chrono>
#include <cstddef>

namespace cucumber_cpp::library::util
{
    namespace
    {
        std::chrono::milliseconds ToMillis(std::chrono::seconds seconds, std::chrono::nanoseconds nanos)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(seconds) +
                   std::chrono::duration_cast<std::chrono::milliseconds>(nanos);
        }

        std::chrono::nanoseconds ToNanoSeconds(std::chrono::seconds seconds, std::chrono::nanoseconds nanos)
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(seconds) + nanos;
        }

        cucumber::messages::duration ToDuration(std::chrono::milliseconds millis)
        {
            return {
                .seconds = millis.count() / millisecondsPerSecond,
                .nanos = (millis.count() % millisecondsPerSecond) * nanosecondsPerMillisecond,
            };
        }
    }

    Stopwatch::Stopwatch()
    {
        Stopwatch::instance = this;
    }

    Stopwatch& Stopwatch::Instance()
    {
        return *instance;
    }

    std::chrono::high_resolution_clock::time_point StopWatchHighResolutionClock::Start()
    {
        return std::chrono::high_resolution_clock::now();
    }

    std::chrono::nanoseconds StopWatchHighResolutionClock::Duration(std::chrono::high_resolution_clock::time_point timePoint)
    {
        const auto timeStop = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(timeStop - timePoint);
    }

    cucumber::messages::duration MillisecondsToDuration(std::chrono::milliseconds millis)
    {
        return ToDuration(millis);
    }

    std::chrono::milliseconds DurationToMilliseconds(const cucumber::messages::duration& duration)
    {
        return ToMillis(std::chrono::seconds{ duration.seconds }, std::chrono::nanoseconds{ duration.nanos });
    }

    std::chrono::nanoseconds DurationToNanoSeconds(const cucumber::messages::duration& duration)
    {
        return ToNanoSeconds(std::chrono::seconds{ duration.seconds }, std::chrono::nanoseconds{ duration.nanos });
    }

    cucumber::messages::duration& operator+=(cucumber::messages::duration& lhs, const cucumber::messages::duration& rhs)
    {
        const auto totalNanos = lhs.nanos + rhs.nanos;
        lhs.seconds += rhs.seconds;
        lhs.seconds += totalNanos / nanosecondsPerSecond;
        lhs.nanos = totalNanos % nanosecondsPerSecond;

        return lhs;
    }

    cucumber::messages::duration operator+(const cucumber::messages::duration& lhs, const cucumber::messages::duration& rhs)
    {
        cucumber::messages::duration result = lhs;

        return result += rhs;
    }
}
