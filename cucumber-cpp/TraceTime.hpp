#ifndef CUCUMBER_CPP_TRACETIME_HPP
#define CUCUMBER_CPP_TRACETIME_HPP

#include <chrono>
#include <ratio>

namespace cucumber_cpp
{
    struct TraceTime
    {
        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
        using Duration = TimePoint::duration;

        struct Scoped
        {
            explicit Scoped(TraceTime& traceTime);
            ~Scoped();

            Scoped(const Scoped&) = delete;
            Scoped& operator=(const Scoped&) = delete;

        private:
            TraceTime& traceTime;
        };

        void Start();
        void Stop();

        [[nodiscard]] Duration Delta() const;

    private:
        TimePoint timeStart{};
        TimePoint timeStop{};
    };
}

#endif
