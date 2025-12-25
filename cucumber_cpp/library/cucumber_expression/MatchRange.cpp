
#include "cucumber_cpp/library/cucumber_expression/MatchRange.hpp"
#include <regex>
#include <utility>

namespace cucumber_cpp::library::cucumber_expression
{

    std::smatch::const_iterator MatchRange::begin() const
    {
        return first;
    }

    std::smatch::const_iterator MatchRange::end() const
    {
        return second;
    }

    const std::ssub_match& MatchRange::operator[](std::size_t index) const
    {
        return *std::next(begin(), index);
    }
}
