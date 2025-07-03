#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "gtest/gtest.h"
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace cucumber_cpp::library
{
    struct TestSteps : testing::Test
    {

        cucumber_expression::ParameterRegistry parameterRegistry;
        StepRegistry stepRegistry{ parameterRegistry };
    };

    TEST_F(TestSteps, RegisterThroughPreregistration)
    {
        EXPECT_THAT(stepRegistry.Size(), testing::Ge(1));
    }

    TEST_F(TestSteps, GetGivenStep)
    {
        const auto matches = stepRegistry.Query("This is a GIVEN step");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("^This is a GIVEN step$"));
    }

    TEST_F(TestSteps, GetWhenStep)
    {
        const auto matches = stepRegistry.Query("This is a WHEN step");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("^This is a WHEN step$"));
    }

    TEST_F(TestSteps, GetThenStep)
    {
        const auto matches = stepRegistry.Query("This is a THEN step");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("^This is a THEN step$"));
    }

    TEST_F(TestSteps, GetAnyStep)
    {
        const auto matches = stepRegistry.Query("This is a STEP step");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("^This is a STEP step$"));
    }

    TEST_F(TestSteps, GetStepWithMatches)
    {
        const auto matches = stepRegistry.Query("This is a step with a 10s delay");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("^This is a step with a ([0-9]+)s delay$"));

        EXPECT_THAT(std::get<std::vector<std::string>>(matches.matches).size(), testing::Eq(1));
        EXPECT_THAT(std::get<std::vector<std::string>>(matches.matches)[0], testing::StrEq("10"));
    }

    TEST_F(TestSteps, GetInvalidStep)
    {
        EXPECT_THROW((void)stepRegistry.Query("This step does not exist"), StepRegistry::StepNotFoundError);
    }

    TEST_F(TestSteps, GetAmbiguousStep)
    {
        EXPECT_THROW((void)stepRegistry.Query("an ambiguous step"), StepRegistry::AmbiguousStepError);
    }

    TEST_F(TestSteps, InvokeTestWithCucumberExpressions)
    {
        const auto matches = stepRegistry.Query(R"(Step with cucumber expression syntax 1.5 "abcdef" 10)");

        auto contextStorage{ std::make_shared<ContextStorageFactoryImpl>() };
        Context context{ contextStorage };

        matches.factory(context, {})->Execute(matches.matches);

        EXPECT_THAT(context.Contains("float"), testing::IsTrue());
        EXPECT_THAT(context.Contains("std::string"), testing::IsTrue());
        EXPECT_THAT(context.Contains("std::int32_t"), testing::IsTrue());

        EXPECT_THAT(context.Get<float>("float"), testing::FloatEq(1.5));
        EXPECT_THAT(context.Get<std::string>("std::string"), testing::StrEq("abcdef"));
        EXPECT_THAT(context.Get<std::int32_t>("std::int32_t"), testing::Eq(10));
    }

    TEST_F(TestSteps, EscapedCucumberExpression)
    {
        const auto matchesParens = stepRegistry.Query(R"(An expression with (parenthesis) should remain as is)");
        EXPECT_THAT(matchesParens.stepRegexStr, testing::StrEq(R"(^An expression with \(parenthesis\) should remain as is$)"));

        const auto matchesBrace = stepRegistry.Query(R"(An expression with {braces} should remain as is)");
        EXPECT_THAT(matchesBrace.stepRegexStr, testing::StrEq(R"(^An expression with \{braces\} should remain as is$)"));
    }

    TEST_F(TestSteps, FunctionLikeStep)
    {
        const auto matches = stepRegistry.Query(R"(An expression that looks like a function func(1, 2) should keep its parameters)");

        auto contextStorage{ std::make_shared<ContextStorageFactoryImpl>() };
        Context context{ contextStorage };

        matches.factory(context, {})->Execute(matches.matches);
    }

    TEST_F(TestSteps, EscapedParenthesis)
    {
        const auto matches1 = stepRegistry.Query(R"(An expression with \(escaped parenthesis\) should keep the slash)");
        const auto matches2 = stepRegistry.Query(R"(An expression with \{escaped braces\} should keep the slash)");
    }
}
