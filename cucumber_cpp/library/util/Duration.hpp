#ifndef UTIL_DURATION_HPP
#define UTIL_DURATION_HPP

#include "cucumber/messages/duration.hpp"
#include <chrono>

namespace cucumber_cpp::library::util
{
    cucumber::messages::duration MillisecondsToDuration(std::chrono::milliseconds millis);

    std::chrono::milliseconds DurationToMilliseconds(const cucumber::messages::duration& duration);
    std::chrono::nanoseconds DurationToNanoSeconds(const cucumber::messages::duration& duration);
    cucumber::messages::duration& operator+=(cucumber::messages::duration& lhs, const cucumber::messages::duration& rhs);
    cucumber::messages::duration operator+(const cucumber::messages::duration& lhs, const cucumber::messages::duration& rhs);

    struct Stopwatch
    {
    protected:
        Stopwatch();
        ~Stopwatch() = default;

    public:
        static Stopwatch& Instance();

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
