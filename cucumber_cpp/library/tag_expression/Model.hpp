#ifndef TAG_EXPRESSION_MODEL_HPP
#define TAG_EXPRESSION_MODEL_HPP

#include "cucumber/messages/pickle_tag.hpp"
#include "cucumber/messages/tag.hpp"
#include <functional>
#include <memory>
#include <set>
#include <span>
#include <string>

namespace cucumber_cpp::library::tag_expression
{
    struct Expression
    {
        virtual ~Expression() = default;

        virtual bool Evaluate(const std::set<std::string, std::less<>>& tags) const = 0;
        virtual bool Evaluate(std::span<const cucumber::messages::pickle_tag> tags) const = 0;
        virtual bool Evaluate(std::span<const cucumber::messages::tag> tags) const = 0;

        virtual explicit operator std::string() const = 0;
    };

    struct TrueExpression : Expression
    {
        bool Evaluate(const std::set<std::string, std::less<>>& tags) const override;
        bool Evaluate(std::span<const cucumber::messages::pickle_tag> tags) const override;
        bool Evaluate(std::span<const cucumber::messages::tag> tags) const override;

        explicit operator std::string() const override;
    };

    struct LiteralExpression : Expression
    {
        explicit LiteralExpression(std::string name);

        bool Evaluate(const std::set<std::string, std::less<>>& tags) const override;
        bool Evaluate(std::span<const cucumber::messages::pickle_tag> tags) const override;
        bool Evaluate(std::span<const cucumber::messages::tag> tags) const override;

        explicit operator std::string() const override;

    private:
        std::string name;
    };

    struct AndExpression : Expression
    {
        AndExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);

        bool Evaluate(const std::set<std::string, std::less<>>& tags) const override;
        bool Evaluate(std::span<const cucumber::messages::pickle_tag> tags) const override;
        bool Evaluate(std::span<const cucumber::messages::tag> tags) const override;

        explicit operator std::string() const override;

    private:
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
    };

    struct OrExpression : Expression
    {
        OrExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);

        bool Evaluate(const std::set<std::string, std::less<>>& tags) const override;
        bool Evaluate(std::span<const cucumber::messages::pickle_tag> tags) const override;
        bool Evaluate(std::span<const cucumber::messages::tag> tags) const override;

        explicit operator std::string() const override;

    private:
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
    };

    struct NotExpression : Expression
    {
        explicit NotExpression(std::unique_ptr<Expression> operand);

        bool Evaluate(const std::set<std::string, std::less<>>& tags) const override;
        bool Evaluate(std::span<const cucumber::messages::pickle_tag> tags) const override;
        bool Evaluate(std::span<const cucumber::messages::tag> tags) const override;

        explicit operator std::string() const override;

    private:
        std::unique_ptr<Expression> operand;
    };
}

#endif
