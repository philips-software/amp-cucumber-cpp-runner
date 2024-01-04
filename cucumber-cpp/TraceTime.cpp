
#include "cucumber-cpp/TraceTime.hpp"
#include "nlohmann/json.hpp"

namespace cucumber_cpp
{
    TraceTime::TraceTime(nlohmann::json& json)
        : json{ json }
        , timeStart{ std::chrono::high_resolution_clock::now() }
    {
    }

    TraceTime::~TraceTime()
    {
        const auto timeEnd = std::chrono::high_resolution_clock::now();

        json["elapsed"] = std::chrono::duration<double, std::ratio<1>>(timeEnd - timeStart).count();
    }
}
