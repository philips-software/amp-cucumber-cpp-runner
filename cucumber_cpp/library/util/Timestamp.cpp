
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/Timestamp.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "fmt/format.h"
#include <chrono>
#include <ctime>
#include <string>

namespace cucumber_cpp::library::util
{
    namespace
    {
        std::chrono::milliseconds ToMillis(std::chrono::seconds seconds, std::chrono::nanoseconds nanos)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(seconds) +
                   std::chrono::duration_cast<std::chrono::milliseconds>(nanos);
        }

        std::chrono::milliseconds TimestampToMillis(const cucumber::messages::Timestamp& timestamp)
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
        if (instance == this)
            instance = nullptr;
    }

    TimestampGenerator& TimestampGenerator::Instance()
    {
        return *instance;
    }

    void TimestampGenerator::SetInstance(TimestampGenerator& inst)
    {
        instance = &inst;
    }

    std::chrono::milliseconds TimestampGeneratorSystemClock::Now()
    {
        const auto now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now);
    }

    cucumber::messages::Timestamp TimestampNow()
    {
        const auto nowMillis = TimestampGenerator::Instance().Now().count();
        const auto seconds = nowMillis / millisecondsPerSecond;
        const auto nanos = (nowMillis % millisecondsPerSecond) * nanosecondsPerMillisecond;
        return cucumber::messages::Timestamp{
            .seconds = seconds,
            .nanos = nanos,
        };
    }

    cucumber::messages::Duration operator-(const cucumber::messages::Timestamp& lhs, const cucumber::messages::Timestamp& rhs) // NOSONAR: cannot be a hidden friend of the generated messages::Timestamp type
    {
        const auto durationMillis = TimestampToMillis(lhs) - TimestampToMillis(rhs);
        return MillisecondsToDuration(durationMillis);
    }

    std::string MakeIso8601Timestamp(const cucumber::messages::Timestamp& timestamp)
    {
        const std::time_t seconds = timestamp.seconds;
        std::tm utcTime{};
    #ifdef _WIN32
        gmtime_s(&utcTime, &seconds);
    #else
        gmtime_r(&seconds, &utcTime);
    #endif
        const auto timestampWithoutFraction = fmt::format(
            "{:04}-{:02}-{:02}T{:02}:{:02}:{:02}",
            utcTime.tm_year + 1900,
            utcTime.tm_mon + 1,
            utcTime.tm_mday,
            utcTime.tm_hour,
            utcTime.tm_min,
            utcTime.tm_sec);
        if (timestamp.nanos == 0)
            return timestampWithoutFraction + "Z";
        return fmt::format("{}.{:09}Z", timestampWithoutFraction, timestamp.nanos);
    }
}
