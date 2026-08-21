
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/Timestamp.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "fmt/chrono.h"
#include "fmt/format.h"
#include <chrono>
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

    cucumber::messages::Duration operator-(const cucumber::messages::Timestamp& lhs, const cucumber::messages::Timestamp& rhs)
    {
        const auto durationMillis = TimestampToMillis(lhs) - TimestampToMillis(rhs);
        return MillisecondsToDuration(durationMillis);
    }

    std::string MakeIso8601Timestamp(const cucumber::messages::Timestamp& timestamp)
    {
        const auto duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::seconds(timestamp.seconds) + std::chrono::nanoseconds(timestamp.nanos));
        const std::chrono::system_clock::time_point tp{ duration };
        return fmt::format("{:%FT%T%Z}", tp);
    }
}
