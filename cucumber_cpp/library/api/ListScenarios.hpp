#ifndef API_LIST_SCENARIOS_HPP
#define API_LIST_SCENARIOS_HPP

#include "cucumber_cpp/library/support/Types.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include <string>
#include <vector>

namespace cucumber_cpp::library::api
{
    std::vector<std::string> ListScenarios(const support::RunOptions::Sources& sources, util::Broadcaster& broadcaster);
}

#endif
