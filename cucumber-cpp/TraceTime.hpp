#ifndef CUCUMBER_CPP_TRACETIME_HPP
#define CUCUMBER_CPP_TRACETIME_HPP

#include "nlohmann/json_fwd.hpp"
#include <chrono>
#include <ratio>

namespace cucumber_cpp
{
    struct TraceTime
    {
        TraceTime(nlohmann::json& json);
        ~TraceTime();

    private:
        nlohmann::json& json;
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart;
    };
}

#endif
