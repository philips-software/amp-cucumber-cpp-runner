#ifndef CUCUMBER_CPP_TAGEXPRESSION_HPP
#define CUCUMBER_CPP_TAGEXPRESSION_HPP

#include <functional>
#include <set>
#include <string>
#include <string_view>

namespace cucumber_cpp::library
{
    bool IsTagExprSelected(std::string_view tagExpr, const std::set<std::string, std::less<>>& tags);
}

#endif
