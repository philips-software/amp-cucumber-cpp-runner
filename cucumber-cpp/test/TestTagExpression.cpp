#include "cucumber-cpp/TagExpression.hpp"
#include "gmock/gmock.h"
#include <iomanip>

namespace cucumber_cpp
{
    struct TestTagExpression : testing::Test
    {
        std::set<std::string, std::less<>> inputTags = { "@abc", "@def", "@efg" };
        std::set<std::string, std::less<>> noTags = {};
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

        EXPECT_THAT(IsTagExprSelected("@foo", inputTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, AndTag)
    {
        EXPECT_THAT(IsTagExprSelected("@abc and @def", noTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and @def", inputTags), testing::IsTrue());

        EXPECT_THAT(IsTagExprSelected("@abc and @foo", inputTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, OrTag)
    {
        EXPECT_THAT(IsTagExprSelected("@foo or @def", noTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc or @foo", inputTags), testing::IsTrue());

        EXPECT_THAT(IsTagExprSelected("@fez or @foo", inputTags), testing::IsFalse());
    }

    TEST_F(TestTagExpression, AndOrGroupedTag)
    {
        EXPECT_THAT(IsTagExprSelected("@abc and (@def or @efg)", noTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and (@def or @efg)", inputTags), testing::IsTrue());

        EXPECT_THAT(IsTagExprSelected("@fez and (@def or @efg)", inputTags), testing::IsFalse());
        EXPECT_THAT(IsTagExprSelected("@abc and (@fez or @bar)", inputTags), testing::IsFalse());
    }
}
