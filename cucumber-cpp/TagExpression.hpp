#ifndef CUCUMBER_CPP_TAGEXPRESSION_HPP
#define CUCUMBER_CPP_TAGEXPRESSION_HPP

#include "nlohmann/json_fwd.hpp"
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp
{
    bool IsTagExprSelected(const std::string& tagExpr, const std::set<std::string>& tags);
    bool IsTagExprSelected(const std::string& tagExpr, const nlohmann::json& json);
}

#endif
