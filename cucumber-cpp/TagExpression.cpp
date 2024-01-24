#ifndef CUCUMBER_CPP_TAGEXPRESSION_CPP
#define CUCUMBER_CPP_TAGEXPRESSION_CPP
#include "cucumber-cpp/TagExpression.hpp"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <ostream>
#include <ranges>
#include <regex>
#include <stdexcept>

namespace cucumber_cpp
{
    namespace
    {
        void Replace(std::string& str, const std::string& from, const std::string& to)
        {
            str = std::regex_replace(str, std::regex(from), to);
        }
    }

    bool IsTagExprSelected(const std::string& tagExpr, const std::set<std::string>& tags)
    {
        if (tagExpr.empty())
        {
            return true;
        }

        if (tags.empty())
        {
            return false;
        }

        std::string eval = tagExpr;

        for (std::smatch matches; std::regex_search(eval, matches, std::regex(R"((@[^ \)]+))"));)
        {
            Replace(eval, matches[1], tags.contains(matches[1]) ? "1" : "0");
        }

        Replace(eval, "not", "!");
        Replace(eval, "or", "|");
        Replace(eval, "and", "&");
        Replace(eval, " ", "");

        for (;;)
        {
            auto s = eval.size();

            Replace(eval, R"(\(0\))", "0");
            Replace(eval, R"(\(1\))", "1");
            Replace(eval, "!0", "1");
            Replace(eval, "!1", "0");

            Replace(eval, "0&0", "0");
            Replace(eval, "0&1", "0");
            Replace(eval, "1&0", "0");
            Replace(eval, "1&1", "1");
            Replace(eval, "00", "0");
            Replace(eval, "01", "0");
            Replace(eval, "10", "0");
            Replace(eval, "11", "1");

            Replace(eval, R"(0\|0)", "0");
            Replace(eval, R"(0\|1)", "1");
            Replace(eval, R"(1\|0)", "1");
            Replace(eval, R"(1\|1)", "1");

            if (s == eval.size())
            {
                break;
            }
        }

        if (eval.size() != 1)
        {
            throw std::logic_error("Could not parse tag expression: \"" + tagExpr + "\"");
        }

        return eval == std::string("1");
    }
}

#endif
