#ifndef CUCUMBER_CPP_JSONTAGTOSET_HPP
#define CUCUMBER_CPP_JSONTAGTOSET_HPP

#include "nlohmann/json_fwd.hpp"
#include <set>
#include <string>

namespace cucumber_cpp
{
    std::set<std::string> JsonTagsToSet(const nlohmann::json& json);
}

#endif
