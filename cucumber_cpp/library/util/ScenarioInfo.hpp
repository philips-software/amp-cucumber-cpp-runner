#ifndef UTIL_SCENARIO_INFO_HPP
#define UTIL_SCENARIO_INFO_HPP

#include <functional>
#include <set>
#include <string>

namespace cucumber_cpp::library::util
{
    struct ScenarioInfo
    {
        std::string name;
        std::set<std::string, std::less<>> tags;
    };
}

#endif
