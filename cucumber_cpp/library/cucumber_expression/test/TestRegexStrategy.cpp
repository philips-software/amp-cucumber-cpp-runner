#include "cucumber_cpp/library/cucumber_expression/Re2RegexStrategy.hpp"
#include "cucumber_cpp/library/cucumber_expression/StdRegexStrategy.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    template<typename T>
    struct TestRegexStrategy : testing::Test
    {};

    using RegexStrategyTypes = testing::Types<StdRegexStrategy, Re2RegexStrategy>;
    TYPED_TEST_SUITE(TestRegexStrategy, RegexStrategyTypes);

    TYPED_TEST(TestRegexStrategy, ReturnsNulloptWhenNoMatch)
    {
        TypeParam strategy{ R"__(hello)__" };

        EXPECT_THAT(strategy.Match("world"), testing::IsFalse());
    }

    TYPED_TEST(TestRegexStrategy, ReturnsVectorWhenPatternMatches)
    {
        TypeParam strategy{ R"__((\d+))__" };

        const auto result = strategy.Match("abc 42 def");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(0), testing::StrEq("42")); // whole match
        EXPECT_THAT(result->at(1), testing::StrEq("42")); // first capture group
    }

    TYPED_TEST(TestRegexStrategy, WholeMatchIsAtIndexZero)
    {
        TypeParam strategy{ R"__((\w+)\s+(\w+))__" };

        const auto result = strategy.Match("hello world");

        ASSERT_THAT(result, testing::IsTrue());
        ASSERT_THAT(result->size(), testing::Eq(3));
        EXPECT_THAT(result->at(0), testing::StrEq("hello world")); // whole match
        EXPECT_THAT(result->at(1), testing::StrEq("hello"));       // first capture group
        EXPECT_THAT(result->at(2), testing::StrEq("world"));       // second capture group
    }

    TYPED_TEST(TestRegexStrategy, MatchesNegativeInteger)
    {
        TypeParam strategy{ R"__((-?\d+))__" };

        const auto result = strategy.Match("-22");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(1), testing::StrEq("-22"));
    }

    TYPED_TEST(TestRegexStrategy, MatchesEmptyCapture)
    {
        TypeParam strategy{ R"__(^The value equals "([^"]*)"$)__" };

        const auto result = strategy.Match(R"__(The value equals "")__");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(1), testing::StrEq(""));
    }

    TEST(Re2RegexStrategy, ThrowsOnInvalidPattern)
    {
        EXPECT_THROW(Re2RegexStrategy{ R"__([invalid)__" }, std::invalid_argument);
    }
}

