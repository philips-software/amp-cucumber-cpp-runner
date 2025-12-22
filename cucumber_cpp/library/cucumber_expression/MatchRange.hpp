#ifndef CUCUMBER_EXPRESSION_MATCH_RANGE_HPP
#define CUCUMBER_EXPRESSION_MATCH_RANGE_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include <regex>
#include <utility>

namespace cucumber_cpp::library::cucumber_expression
{
    struct MatchRange : std::pair<std::smatch::const_iterator, std::smatch::const_iterator>
    {
        using std::pair<std::smatch::const_iterator, std::smatch::const_iterator>::pair;

        std::smatch::const_iterator begin() const;
        std::smatch::const_iterator end() const;

        const std::ssub_match& operator[](std::size_t index) const;
    };
}

#endif
