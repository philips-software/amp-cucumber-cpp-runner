
#include "cucumber-cpp/TraceTime.hpp"

namespace cucumber_cpp
{

    TraceTime::Scoped::Scoped(TraceTime& traceTime)
        : traceTime{ traceTime }
    {
        traceTime.Start();
    }

    TraceTime::Scoped::~Scoped()
    {
        traceTime.Stop();
    }

    void TraceTime::Start()
    {
        timeStart = std::chrono::high_resolution_clock::now();
    }

    void TraceTime::Stop()
    {
        timeStop = std::chrono::high_resolution_clock::now();
    }

    TraceTime::Duration TraceTime::Delta() const
    {
        if (timeStop != TimePoint{})
            return timeStop - timeStart;

        return std::chrono::high_resolution_clock::now() - timeStart;
    }
}
