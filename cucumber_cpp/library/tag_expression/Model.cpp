#include "cucumber_cpp/library/tag_expression/Model.hpp"
#include <cstddef>
#include <format>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <utility>

namespace cucumber_cpp::library::tag_expression
{
    bool TrueExpression::Evaluate(const std::set<std::string, std::less<>>& tags) const
    {
        return true;
    }

    TrueExpression::operator std::string() const
    {
        return "true";
    }

    LiteralExpression::LiteralExpression(std::string name)
        : name{ std::move(name) }
    {}

    bool LiteralExpression::Evaluate(const std::set<std::string, std::less<>>& tags) const
    {
        return tags.contains(name);
    }

    LiteralExpression::operator std::string() const
    {
        auto replaceAll = [](std::string& str, const std::string& from, const std::string& to)
        {
            if (from.empty())
                return;
            std::size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos)
            {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
        };

        auto copy = name;

        replaceAll(copy, "\\", "\\\\");
        replaceAll(copy, "(", "\\(");
        replaceAll(copy, ")", "\\)");

        copy = std::regex_replace(copy, std::regex(R"((\s))"), R"(\$&)");

        return copy;
    }

    AndExpression::AndExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : left{ std::move(left) }
        , right{ std::move(right) }
    {}

    bool AndExpression::Evaluate(const std::set<std::string, std::less<>>& tags) const
    {
        return left->Evaluate(tags) && right->Evaluate(tags);
    }

    AndExpression::operator std::string() const
    {
        if (!left || !right)
            return "";

        return std::format(R"(( {} and {} ))", static_cast<std::string>(*left), static_cast<std::string>(*right));
    }

    OrExpression::OrExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : left{ std::move(left) }
        , right{ std::move(right) }
    {}

    bool OrExpression::Evaluate(const std::set<std::string, std::less<>>& tags) const
    {
        return left->Evaluate(tags) || right->Evaluate(tags);
    }

    OrExpression::operator std::string() const
    {
        if (!left || !right)
            return "";

        return std::format(R"(( {} or {} ))", static_cast<std::string>(*left), static_cast<std::string>(*right));
    }

    NotExpression::NotExpression(std::unique_ptr<Expression> operand)
        : operand{ std::move(operand) }
    {}

    bool NotExpression::Evaluate(const std::set<std::string, std::less<>>& tags) const
    {
        return !operand->Evaluate(tags);
    }

    NotExpression::operator std::string() const
    {
        if (!operand)
            return "";

        if (const auto& ref = *operand.get(); typeid(ref) == typeid(AndExpression) || typeid(ref) == typeid(OrExpression))
            return std::format(R"(not {})", static_cast<std::string>(*operand));

        return std::format(R"(not ( {} ))", static_cast<std::string>(*operand));
    }
}
