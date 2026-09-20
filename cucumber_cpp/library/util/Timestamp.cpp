
#include "cucumber_cpp/library/util/Timestamp.hpp"
#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/Timestamp.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
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

    cucumber::messages::Duration operator-(const cucumber::messages::Timestamp& lhs, const cucumber::messages::Timestamp& rhs) // NOSONAR: cannot be a hidden friend of the generated messages::Timestamp type
    {
        const auto durationMillis = TimestampToMillis(lhs) - TimestampToMillis(rhs);
        return MillisecondsToDuration(durationMillis);
    }

    std::string MakeIso8601Timestamp(const cucumber::messages::Timestamp& timestamp)
    {
        const auto seconds = std::chrono::seconds(timestamp.seconds);
        const auto days = std::chrono::floor<std::chrono::days>(seconds);
        const auto timeOfDay = seconds - days;
        auto civilDays = days.count() + 719468;
        const auto era = (civilDays >= 0 ? civilDays : civilDays - 146096) / 146097;
        const auto dayOfEra = civilDays - era * 146097;
        const auto yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
        auto year = yearOfEra + era * 400;
        const auto dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
        const auto monthPart = (5 * dayOfYear + 2) / 153;
        const auto day = dayOfYear - (153 * monthPart + 2) / 5 + 1;
        const auto month = monthPart + (monthPart < 10 ? 3 : -9);
        year += month <= 2;
        const auto timestampWithoutFraction = fmt::format(
            "{:04}-{:02}-{:02}T{:02}:{:02}:{:02}",
            year,
            month,
            day,
            std::chrono::duration_cast<std::chrono::hours>(timeOfDay).count(),
            std::chrono::duration_cast<std::chrono::minutes>(timeOfDay).count() % 60,
            timeOfDay.count() % 60);
        if (timestamp.nanos == 0)
            return timestampWithoutFraction + "Z";
        return fmt::format("{}.{:09}Z", timestampWithoutFraction, timestamp.nanos);
    }
}
