
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber/messages/Duration.hpp"
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

        cucumber::messages::Duration ToDuration(std::chrono::milliseconds millis)
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

    void Stopwatch::SetInstance(Stopwatch& inst)
    {
        instance = &inst;
    }

    Stopwatch::~Stopwatch()
    {
        if (instance == this)
            instance = nullptr;
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

    cucumber::messages::Duration MillisecondsToDuration(std::chrono::milliseconds millis)
    {
        return ToDuration(millis);
    }

    std::chrono::milliseconds DurationToMilliseconds(const cucumber::messages::Duration& duration)
    {
        return ToMillis(std::chrono::seconds{ duration.seconds }, std::chrono::nanoseconds{ duration.nanos });
    }

    std::chrono::nanoseconds DurationToNanoSeconds(const cucumber::messages::Duration& duration)
    {
        return ToNanoSeconds(std::chrono::seconds{ duration.seconds }, std::chrono::nanoseconds{ duration.nanos });
    }

    cucumber::messages::Duration& operator+=(cucumber::messages::Duration& lhs, const cucumber::messages::Duration& rhs)
    {
        const auto totalNanos = lhs.nanos + rhs.nanos;
        lhs.seconds += rhs.seconds;
        lhs.seconds += totalNanos / nanosecondsPerSecond;
        lhs.nanos = totalNanos % nanosecondsPerSecond;

        return lhs;
    }

    cucumber::messages::Duration operator+(const cucumber::messages::Duration& lhs, const cucumber::messages::Duration& rhs)
    {
        cucumber::messages::Duration result = lhs;

        return result += rhs;
    }
}
