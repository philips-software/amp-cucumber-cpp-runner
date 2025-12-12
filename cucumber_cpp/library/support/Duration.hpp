#ifndef SUPPORT_DURATION_HPP
#define SUPPORT_DURATION_HPP

#include "cucumber/messages/duration.hpp"
#include <chrono>

namespace cucumber_cpp::library::support
{
    cucumber::messages::duration MillisecondsToDuration(std::chrono::milliseconds millis);

    std::chrono::milliseconds DurationToMilliseconds(const cucumber::messages::duration& duration);
    cucumber::messages::duration& operator+=(cucumber::messages::duration& lhs, const cucumber::messages::duration& rhs);

    struct Stopwatch
    {
    protected:
        Stopwatch();
        ~Stopwatch() = default;

    public:
        static Stopwatch& Instance();

        virtual void Start() = 0;
        virtual std::chrono::nanoseconds Duration() = 0;

    private:
        static inline Stopwatch* instance{ nullptr };
    };

    struct StopWatchHighResolutionClock : Stopwatch
    {
        virtual ~StopWatchHighResolutionClock() = default;
        void Start() override;
        std::chrono::nanoseconds Duration() override;

    private:
        std::chrono::high_resolution_clock::time_point timeStart{};
    };
}

namespace cucumber::messages
{
    using cucumber_cpp::library::support::operator+=;
};

#endif
