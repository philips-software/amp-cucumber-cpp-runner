#ifndef CUCUMBER_CPP_TRACETIME_HPP
#define CUCUMBER_CPP_TRACETIME_HPP

#include <chrono>

namespace cucumber_cpp
{
    struct TraceTime
    {
        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
        using Duration = TimePoint::duration;

        void Start();
        void Stop();

        [[nodiscard]] Duration Delta() const;

    private:
        TimePoint timeStart{};
        TimePoint timeStop{};
    };
}

#endif
