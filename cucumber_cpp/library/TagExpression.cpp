#include "cucumber_cpp/library/TagExpression.hpp"
#include "cucumber_cpp/library/InternalError.hpp"
#include <functional>
#include <regex>
#include <set>
#include <string>
#include <string_view>

namespace cucumber_cpp::library
{
    namespace
    {
        void Replace(std::string& str, const std::string& from, const std::string& to)
        {
            str = std::regex_replace(str, std::regex(from), to);
        }
    }

    bool IsTagExprSelected(std::string_view tagExpr, const std::set<std::string, std::less<>>& tags)
    {
        if (tagExpr.empty())
        {
            return true;
        }

        std::string eval{ tagExpr };

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
            throw InternalError("Could not parse tag expression: \"" + std::string{ tagExpr } + "\"");
        }

        return eval == std::string("1");
    }
}
