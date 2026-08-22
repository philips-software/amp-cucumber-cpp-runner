#ifndef UTIL_DURATION_HPP
#define UTIL_DURATION_HPP

#include "cucumber/messages/Duration.hpp"
#include <chrono>

namespace cucumber_cpp::library::util
{
    cucumber::messages::Duration MillisecondsToDuration(std::chrono::milliseconds millis);

    std::chrono::milliseconds DurationToMilliseconds(const cucumber::messages::Duration& duration);
    std::chrono::nanoseconds DurationToNanoSeconds(const cucumber::messages::Duration& duration);
    cucumber::messages::Duration& operator+=(cucumber::messages::Duration& lhs, const cucumber::messages::Duration& rhs);
    cucumber::messages::Duration operator+(const cucumber::messages::Duration& lhs, const cucumber::messages::Duration& rhs);

    struct Stopwatch
    {
        Stopwatch();
        virtual ~Stopwatch();

        Stopwatch(const Stopwatch&) = delete;
        Stopwatch& operator=(const Stopwatch&) = delete;
        Stopwatch(Stopwatch&&) = delete;
        Stopwatch& operator=(Stopwatch&&) = delete;

        static Stopwatch& Instance();
        static void SetInstance(Stopwatch& inst);

        virtual std::chrono::high_resolution_clock::time_point Start() = 0;
        virtual std::chrono::nanoseconds Duration(std::chrono::high_resolution_clock::time_point timepPoint) = 0;

    private:
        static inline Stopwatch* instance{ nullptr };
    };

    struct StopWatchHighResolutionClock : Stopwatch
    {
        virtual ~StopWatchHighResolutionClock() = default;
        std::chrono::high_resolution_clock::time_point Start() override;
        std::chrono::nanoseconds Duration(std::chrono::high_resolution_clock::time_point timePoint) override;
    };
}

namespace cucumber::messages
{
    using cucumber_cpp::library::util::operator+=;
    using cucumber_cpp::library::util::operator+;
};

#endif
