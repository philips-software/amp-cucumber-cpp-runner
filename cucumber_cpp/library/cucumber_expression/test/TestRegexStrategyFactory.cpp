#include "cucumber_cpp/library/cucumber_expression/RegexStrategyFactory.hpp"
#include "cucumber_cpp/library/cucumber_expression/StdRegexStrategy.hpp"
#ifdef CCR_HAS_RE2
#include "cucumber_cpp/library/cucumber_expression/Re2RegexStrategy.hpp"
#endif
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cucumber_cpp::library::cucumber_expression
{
    TEST(RegexStrategyFactory, ReturnsNonNullStrategy)
    {
        const auto strategy = CreateRegexStrategy(R"__((\d+))__");

        EXPECT_THAT(strategy, testing::NotNull());
    }

    TEST(RegexStrategyFactory, ReturnedStrategyCanMatch)
    {
        const auto strategy = CreateRegexStrategy(R"__((\d+))__");

        const auto result = strategy->Match("abc 42 def");

        ASSERT_THAT(result, testing::IsTrue());
        EXPECT_THAT(result->at(1), testing::StrEq("42"));
    }

#ifdef CCR_HAS_RE2
    TEST(RegexStrategyFactory, ReturnsRe2Strategy)
    {
        const auto strategy = CreateRegexStrategy(R"__((\d+))__");

        EXPECT_THAT(dynamic_cast<Re2RegexStrategy*>(strategy.get()), testing::NotNull());
    }
#else
    TEST(RegexStrategyFactory, ReturnsStdStrategy)
    {
        const auto strategy = CreateRegexStrategy(R"__((\d+))__");

        EXPECT_THAT(dynamic_cast<StdRegexStrategy*>(strategy.get()), testing::NotNull());
    }
#endif
}
