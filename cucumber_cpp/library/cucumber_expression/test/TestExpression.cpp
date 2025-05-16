
#include "cucumber_cpp/library/cucumber_expression/Expression.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "gmock/gmock.h"
#include <any>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <gtest/gtest.h>
#include <iterator>
#include <limits>
#include <optional>
#include <print>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    struct TestExpression : testing::Test
    {
        ParameterRegistry parameterRegistry{};

        std::optional<std::vector<std::any>> Match(std::string expr, std::string text)
        {
            Expression expression{ std::move(expr), parameterRegistry };
            return expression.Match(std::move(text));
        }
    };

    TEST_F(TestExpression, Escape)
    {
        auto match = Match(R"(hello \{world?)", "hello {world?");
        EXPECT_THAT(match, testing::IsTrue());
    }

    TEST_F(TestExpression, MatchOptional)
    {
        Expression expression{ R"(hello (world))", parameterRegistry };
        EXPECT_THAT(expression.Pattern(), testing::StrEq(R"__(^hello (?:world)?$)__"));
        EXPECT_THAT(expression.Match("hello world"), testing::IsTrue());
        EXPECT_THAT(expression.Match("hello "), testing::IsTrue());
    }

    TEST_F(TestExpression, MatchAlternative)
    {
        Expression expression{ R"(hello country/wo\/rld/city)", parameterRegistry };
        EXPECT_THAT(expression.Pattern(), testing::StrEq(R"__(^hello (?:country|wo/rld|city)$)__"));
        EXPECT_THAT(expression.Match("hello country"), testing::IsTrue());
        EXPECT_THAT(expression.Match("hello wo/rld"), testing::IsTrue());
        EXPECT_THAT(expression.Match("hello city"), testing::IsTrue());
    }

    TEST_F(TestExpression, MatchString)
    {
        auto match = Match(R"__(hello {string})__", R"__(hello "beautiful world")__");
        EXPECT_THAT(match, testing::IsTrue());
        EXPECT_THAT(std::any_cast<std::string>((*match)[0]), testing::StrEq(R"__(beautiful world)__"));
    }

    TEST_F(TestExpression, MatchInt)
    {
        auto match = Match(R"__(there are {int} cucumbers)__", R"__(there are 15 cucumbers)__");
        EXPECT_THAT(match, testing::IsTrue());
        EXPECT_THAT(std::any_cast<std::int64_t>((*match)[0]), testing::Eq(15));
    }

    TEST_F(TestExpression, MatchFloat)
    {
        EXPECT_THAT(Match(R"__({float})__", R"__()__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(.)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(,)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(-)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(E)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(,1)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1.)__"), testing::IsFalse());

        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(1)__"))[0]), testing::FloatNear(1.0f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-1)__"))[0]), testing::FloatNear(-1.0f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(1.1)__"))[0]), testing::FloatNear(1.1f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000,0)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000.1)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000,10)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,0.1)__"), testing::IsFalse());
        EXPECT_THAT(Match(R"__({float})__", R"__(1,000,000.1)__"), testing::IsFalse());
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-1.1)__"))[0]), testing::FloatNear(-1.1f, std::numeric_limits<float>::epsilon()));

        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(.1)__"))[0]), testing::FloatNear(0.1f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1)__"))[0]), testing::FloatNear(-0.1f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1000001)__"))[0]), testing::FloatNear(-0.1000001f, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(1E1)__"))[0]), testing::FloatNear(10.0, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(.1E1)__"))[0]), testing::FloatNear(1, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(Match(R"__({float})__", R"__(1,E1)__"), testing::IsFalse());
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.01)__"))[0]), testing::FloatNear(-0.01, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E-1)__"))[0]), testing::FloatNear(-0.01, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E-2)__"))[0]), testing::FloatNear(-0.001, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E+1)__"))[0]), testing::FloatNear(-1, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E+2)__"))[0]), testing::FloatNear(-10, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E1)__"))[0]), testing::FloatNear(-1, std::numeric_limits<float>::epsilon()));
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(-.1E2)__"))[0]), testing::FloatNear(-10, std::numeric_limits<float>::epsilon()));
    }

    TEST_F(TestExpression, FloatWithZero)
    {
        EXPECT_THAT(std::any_cast<float>((*Match(R"__({float})__", R"__(0)__"))[0]), testing::FloatNear(0.0f, std::numeric_limits<float>::epsilon()));
    }

    TEST_F(TestExpression, MatchAnonymous)
    {
        EXPECT_THAT(std::any_cast<std::string>((*Match(R"__({})__", R"__(0.22)__"))[0]), testing::StrEq("0.22"));
    }

    TEST_F(TestExpression, MatchCustom)
    {
        struct CustomType
        {
            std::optional<std::string> text;
            std::optional<std::int64_t> number;
        };

        parameterRegistry.AddParameter("textAndOrNumber", { R"(([A-Z]+)?(?: )?([0-9]+)?)" }, [](MatchRange matches) -> std::any
            {
                std::println("matches: {}", std::ranges::distance(matches));
                for (const auto& match : matches)
                    std::println("match: {}", match.str());

                std::optional<std::string> text{ matches[1].matched ? StringTo<std::string>(matches[1].str()) : std::optional<std::string>{ std::nullopt } };
                std::optional<std::int64_t> number{ matches[2].matched ? StringTo<std::int64_t>(matches[2].str()) : std::optional<std::int64_t>{ std::nullopt } };

                return CustomType{ text, number };
            });

        auto matchString{ Match(R"__({textAndOrNumber})__", R"__(ABC)__") };
        EXPECT_THAT(matchString, testing::IsTrue());
        EXPECT_THAT(std::any_cast<CustomType>((*matchString)[0]).text.value(), testing::StrEq("ABC"));
        EXPECT_THAT(std::any_cast<CustomType>((*matchString)[0]).number, testing::IsFalse());

        auto matchInt{ Match(R"__({textAndOrNumber})__", R"__(123)__") };
        EXPECT_THAT(matchInt, testing::IsTrue());
        EXPECT_THAT(std::any_cast<CustomType>((*matchInt)[0]).text, testing::IsFalse());
        EXPECT_THAT(std::any_cast<CustomType>((*matchInt)[0]).number.value(), testing::Eq(123));

        auto matchStringAndInt{ Match(R"__({textAndOrNumber})__", R"__(ABC 123)__") };
        EXPECT_THAT(matchStringAndInt, testing::IsTrue());
        EXPECT_THAT(std::any_cast<CustomType>((*matchStringAndInt)[0]).text.value(), testing::StrEq("ABC"));
        EXPECT_THAT(std::any_cast<CustomType>((*matchStringAndInt)[0]).number.value(), testing::Eq(123));
    }

    TEST_F(TestExpression, ExposeSource)
    {
        auto expr = "I have {int} cuke(s)";
        Expression expression{ expr, parameterRegistry };
        EXPECT_THAT(expr, testing::StrEq(expression.Source()));
    }

    TEST_F(TestExpression, MultiParamParsing)
    {
        auto matchString{ Match(R"__(Step with cucumber expression syntax {float} {string} {int})__", R"__(Step with cucumber expression syntax 1.1 "string" 10)__") };
        EXPECT_THAT(matchString, testing::IsTrue());
    }
}
