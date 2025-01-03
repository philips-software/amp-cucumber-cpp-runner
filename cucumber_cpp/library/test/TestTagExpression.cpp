#include "cucumber_cpp/library/TagExpression.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <functional>
#include <set>
#include <string>

namespace cucumber_cpp::library
{
    struct TestTagExpression : testing::Test
    {
        std::set<std::string, std::less<>> inputTags = { "@abc", "@def", "@efg" };
        std::set<std::string, std::less<>> noTags = {};
        std::set<std::string, std::less<>> ignoredTags = { "@abc", "@def", "@efg", "@ignore" };
    };

    TEST_F(TestTagExpression, EmptyTagExpression)
    {
        EXPECT_THAT(IsTagExprSelected("", noTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("", inputTags), testing::IsTrue());
    }

    TEST_F(TestTagExpression, EmptyTags)
    {
        EXPECT_THAT(IsTagExprSelected("", noTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("@foo", noTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, SingleTag)
    {
        EXPECT_THAT(IsTagExprSelected("@abc", noTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc", inputTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("@abc", ignoredTags), testing::IsTrue());

        EXPECT_THAT(IsTagExprSelected("@foo", inputTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@foo", ignoredTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, AndTag)
    {
        EXPECT_THAT(IsTagExprSelected("@abc and @def", noTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and @def", inputTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("@abc and @def", ignoredTags), testing::IsTrue());

        EXPECT_THAT(IsTagExprSelected("@abc and @foo", inputTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and @foo", ignoredTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, OrTag)
    {
        EXPECT_THAT(IsTagExprSelected("@foo or @def", noTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc or @foo", inputTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("@abc or @foo", ignoredTags), testing::IsTrue());

        EXPECT_THAT(IsTagExprSelected("@fez or @foo", inputTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@fez or @foo", ignoredTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, AndOrGroupedTag)
    {
        EXPECT_THAT(IsTagExprSelected("@abc and (@def or @efg)", noTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and (@def or @efg)", inputTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("@abc and (@def or @efg)", ignoredTags), testing::IsTrue());

        EXPECT_THAT(IsTagExprSelected("@fez and (@def or @efg)", inputTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@fez and (@def or @efg)", ignoredTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and (@fez or @bar)", inputTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and (@fez or @bar)", ignoredTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, Negating)
    {
        EXPECT_THAT(IsTagExprSelected("not @ignore", noTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("not @ignore", inputTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("not @ignore", ignoredTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, WithTagNegating)
    {
        EXPECT_THAT(IsTagExprSelected("@abc and not @ignore", noTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and not @ignore", inputTags), testing::IsTrue());
        EXPECT_THAT(IsTagExprSelected("@abc and not @ignore", ignoredTags), testing::IsFalse());
    }
}
