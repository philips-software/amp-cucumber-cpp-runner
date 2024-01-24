#ifndef CUCUMBER_CPP_TAGEXPRESSION_HPP
#define CUCUMBER_CPP_TAGEXPRESSION_HPP

#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp
{
    bool IsTagExprSelected(const std::string& tagExpr, const std::set<std::string, std::less<>>& tags);
}

#endif
