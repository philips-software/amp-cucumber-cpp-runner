#ifndef CUCUMBER_EXPRESSION_REGULAREXPRESSION_HPP
#define CUCUMBER_EXPRESSION_REGULAREXPRESSION_HPP

#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    struct RegularExpression
    {
        explicit RegularExpression(std::string expression)
            : expression{ std::move(expression) }
            , regex{ this->expression }
        {}

        std::string_view Source() const
        {
            return expression;
        }

        std::string_view Pattern() const
        {
            return expression;
        }

        std::optional<std::vector<std::string>> Match(const std::string& text) const
        {
            std::smatch smatch;
            if (!std::regex_search(text, smatch, regex))
                return std::nullopt;

            std::vector<std::string> result{};
            result.reserve(smatch.size() - 1);

            for (const auto& match : smatch | std::views::drop(1))
                result.emplace_back(match.str());

            return result;
        }

    private:
        std::string expression;
        std::regex regex;
    };
}

#endif
