#include "cucumber_cpp/library/cucumber_expression/RegexStrategy.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

namespace cucumber_cpp::library::cucumber_expression
{
    // StdRegexStrategy

    TEST(StdRegexStrategy, ReturnsNulloptWhenNoMatch)
    {
        StdRegexStrategy strategy{ R"__(hello)__" };

        EXPECT_THAT(strategy.Match("world"), testing::IsFalse());
    }

    TEST(StdRegexStrategy, ReturnsVectorWhenPatternMatches)
    {
        StdRegexStrategy strategy{ R"__((\d+))__" };

        const auto result = strategy.Match("abc 42 def");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(0), testing::StrEq("42")); // whole match
        EXPECT_THAT(result->at(1), testing::StrEq("42")); // first capture group
    }

    TEST(StdRegexStrategy, WholeMatchIsAtIndexZero)
    {
        StdRegexStrategy strategy{ R"__((\w+)\s+(\w+))__" };

        const auto result = strategy.Match("hello world");

        ASSERT_THAT(result, testing::IsTrue());
        ASSERT_THAT(result->size(), testing::Eq(3));
        EXPECT_THAT(result->at(0), testing::StrEq("hello world")); // whole match
        EXPECT_THAT(result->at(1), testing::StrEq("hello"));       // first capture group
        EXPECT_THAT(result->at(2), testing::StrEq("world"));       // second capture group
    }

    TEST(StdRegexStrategy, MatchesNegativeInteger)
    {
        StdRegexStrategy strategy{ R"__((-?\d+))__" };

        const auto result = strategy.Match("-22");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(1), testing::StrEq("-22"));
    }

    TEST(StdRegexStrategy, MatchesEmptyCapture)
    {
        StdRegexStrategy strategy{ R"__(^The value equals "([^"]*)"$)__" };

        const auto result = strategy.Match(R"__(The value equals "")__");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(1), testing::StrEq(""));
    }

    // Re2RegexStrategy

    TEST(Re2RegexStrategy, ReturnsNulloptWhenNoMatch)
    {
        Re2RegexStrategy strategy{ R"__(hello)__" };

        EXPECT_THAT(strategy.Match("world"), testing::IsFalse());
    }

    TEST(Re2RegexStrategy, ReturnsVectorWhenPatternMatches)
    {
        Re2RegexStrategy strategy{ R"__((\d+))__" };

        const auto result = strategy.Match("abc 42 def");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(0), testing::StrEq("42")); // whole match
        EXPECT_THAT(result->at(1), testing::StrEq("42")); // first capture group
    }

    TEST(Re2RegexStrategy, WholeMatchIsAtIndexZero)
    {
        Re2RegexStrategy strategy{ R"__((\w+)\s+(\w+))__" };

        const auto result = strategy.Match("hello world");

        ASSERT_THAT(result, testing::IsTrue());
        ASSERT_THAT(result->size(), testing::Eq(3));
        EXPECT_THAT(result->at(0), testing::StrEq("hello world")); // whole match
        EXPECT_THAT(result->at(1), testing::StrEq("hello"));       // first capture group
        EXPECT_THAT(result->at(2), testing::StrEq("world"));       // second capture group
    }

    TEST(Re2RegexStrategy, MatchesNegativeInteger)
    {
        Re2RegexStrategy strategy{ R"__((-?\d+))__" };

        const auto result = strategy.Match("-22");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(1), testing::StrEq("-22"));
    }

    TEST(Re2RegexStrategy, MatchesEmptyCapture)
    {
        Re2RegexStrategy strategy{ R"__(^The value equals "([^"]*)"$)__" };

        const auto result = strategy.Match(R"__(The value equals "")__");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(1), testing::StrEq(""));
    }
}
