#ifndef TAG_EXPRESSION_MODEL_HPP
#define TAG_EXPRESSION_MODEL_HPP

#include <cstddef>
#include <format>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <utility>

namespace cucumber_cpp::library::tag_expression
{
    struct Expression
    {
        virtual bool Evaluate(const std::set<std::string>& tags) const = 0;
        virtual operator std::string() const = 0;
    };

    struct TrueExpression : Expression
    {
        bool Evaluate(const std::set<std::string>& tags) const override
        {
            return true;
        }

        operator std::string() const override
        {
            return "true";
        }
    };

    struct LiteralExpression : Expression
    {
        LiteralExpression(std::string name)
            : name{ std::move(name) }
        {}

        bool Evaluate(const std::set<std::string>& tags) const override
        {
            return tags.contains(name);
        }

        operator std::string() const override
        {
            auto replaceAll = [](std::string& str, const std::string& from, const std::string& to)
            {
                if (from.empty())
                    return;
                std::size_t start_pos = 0;
                while ((start_pos = str.find(from, start_pos)) != std::string::npos)
                {
                    str.replace(start_pos, from.length(), to);
                    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
                }
            };

            auto copy = name;

            replaceAll(copy, "\\", "\\\\");
            replaceAll(copy, "(", "\\(");
            replaceAll(copy, ")", "\\)");

            copy = std::regex_replace(copy, std::regex(R"((\s))"), R"(\$&)");

            return copy;
        }

        std::string name;
    };

    struct AndExpression : Expression
    {
        AndExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
            : left{ std::move(left) }
            , right{ std::move(right) }
        {}

        bool Evaluate(const std::set<std::string>& tags) const override
        {
            return left->Evaluate(tags) && right->Evaluate(tags);
        }

        operator std::string() const override
        {
            if (!left || !right)
                return "";

            return std::format(R"(( {} and {} ))", static_cast<std::string>(*left), static_cast<std::string>(*right));
        }

        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
    };

    struct OrExpression : Expression
    {
        OrExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
            : left{ std::move(left) }
            , right{ std::move(right) }
        {}

        bool Evaluate(const std::set<std::string>& tags) const override
        {
            return left->Evaluate(tags) || right->Evaluate(tags);
        }

        operator std::string() const override
        {
            if (!left || !right)
                return "";

            return std::format(R"(( {} or {} ))", static_cast<std::string>(*left), static_cast<std::string>(*right));
        }

        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
    };

    struct NotExpression : Expression
    {
        NotExpression(std::unique_ptr<Expression> operand)
            : operand{ std::move(operand) }
        {}

        bool Evaluate(const std::set<std::string>& tags) const override
        {
            return !operand->Evaluate(tags);
        }

        operator std::string() const override
        {
            if (!operand)
                return "";

            if (const auto& ref = *operand.get(); typeid(ref) == typeid(AndExpression) || typeid(ref) == typeid(OrExpression))
                return std::format(R"(not {})", static_cast<std::string>(*operand));

            return std::format(R"(not ( {} ))", static_cast<std::string>(*operand));
        }

    private:
        std::unique_ptr<Expression> operand;
    };
}

#endif
