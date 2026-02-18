#include "cucumber_cpp/library/cucumber_expression/MatchRange.hpp"
#include <cstddef>
#include <iterator>
#include <regex>

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

    const std::ssub_match& MatchRange::operator[](std::ptrdiff_t index) const
    {
        return *std::next(begin(), index);
    }
}
