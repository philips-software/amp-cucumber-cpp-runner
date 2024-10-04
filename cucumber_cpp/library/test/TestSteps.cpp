#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/Steps.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>

namespace cucumber_cpp
{
    GIVEN("This is a GIVEN step")
    {
        /* do nothing */
    }

    WHEN("This is a WHEN step")
    {
        /* do nothing */
    }

    THEN("This is a THEN step")
    {
        /* do nothing */
    }

    STEP("This is a STEP step")
    {
        /* do nothing */
    }

    STEP("This is a step with a ([0-9]+)s delay", (std::uint32_t delay))
    {
        context.InsertAt("std::uint32_t", delay);
    }

    THEN("an ambiguous step")
    {
        /* do nothing */
    }

    STEP("an ambiguous step")
    {
        /* do nothing */
    }

    STEP("Step with cucumber expression syntax {float} {string} {int}", (float fl, std::string str, std::uint32_t nr))
    {
        context.InsertAt("float", fl);
        context.InsertAt("std::string", str);
        context.InsertAt("std::uint32_t", nr);
    }

    struct TestSteps : testing::Test
    {
        StepRegistry& stepRegistry{ StepRegistry::Instance() };
    };

    TEST_F(TestSteps, RegisterThroughPreregistration)
    {
        EXPECT_THAT(stepRegistry.Size(), testing::Eq(8));
        EXPECT_THAT(stepRegistry.Size(StepType::given), testing::Eq(1));
        EXPECT_THAT(stepRegistry.Size(StepType::when), testing::Eq(1));
        EXPECT_THAT(stepRegistry.Size(StepType::then), testing::Eq(2));
        EXPECT_THAT(stepRegistry.Size(StepType::any), testing::Eq(4));
    }

    TEST_F(TestSteps, GetGivenStep)
    {
        const auto matches = stepRegistry.Query(StepType::given, "This is a GIVEN step");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("This is a GIVEN step"));
    }

    TEST_F(TestSteps, GetWhenStep)
    {
        const auto matches = stepRegistry.Query(StepType::when, "This is a WHEN step");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("This is a WHEN step"));
    }

    TEST_F(TestSteps, GetThenStep)
    {
        const auto matches = stepRegistry.Query(StepType::then, "This is a THEN step");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("This is a THEN step"));
    }

    TEST_F(TestSteps, GetAnyStep)
    {
        const auto matches = stepRegistry.Query(StepType::given, "This is a STEP step");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("This is a STEP step"));
    }

    TEST_F(TestSteps, GetStepWithMatches)
    {
        const auto matches = stepRegistry.Query(StepType::when, "This is a step with a 10s delay");

        EXPECT_THAT(matches.stepRegexStr, testing::StrEq("This is a step with a ([0-9]+)s delay"));

        EXPECT_THAT(matches.matches.size(), testing::Eq(1));
        EXPECT_THAT(matches.matches[0], testing::StrEq("10"));
    }

    TEST_F(TestSteps, GetInvalidStep)
    {
        EXPECT_THROW((void)stepRegistry.Query(StepType::when, "This step does not exist"), StepRegistryBase::StepNotFoundError);
    }

    TEST_F(TestSteps, GetAmbiguousStep)
    {
        EXPECT_NO_THROW((void)stepRegistry.Query(StepType::given, "an ambiguous step"));
        EXPECT_NO_THROW((void)stepRegistry.Query(StepType::when, "an ambiguous step"));

        EXPECT_THROW((void)stepRegistry.Query(StepType::then, "an ambiguous step"), StepRegistryBase::AmbiguousStepError);
    }

    TEST_F(TestSteps, InvokeTestWithCucumberExpressions)
    {
        const auto matches = stepRegistry.Query(StepType::when, R"(Step with cucumber expression syntax 1.5 """abcdef"" 10)");

        auto contextStorage{ std::make_shared<ContextStorageFactoryImpl>() };
        Context context{ contextStorage };

        matches.factory(context, {})->Execute(matches.matches);

        EXPECT_THAT(context.Contains("float"), testing::IsTrue());
        EXPECT_THAT(context.Contains("std::string"), testing::IsTrue());
        EXPECT_THAT(context.Contains("std::uint32_t"), testing::IsTrue());

        EXPECT_THAT(context.Get<float>("float"), testing::FloatEq(1.5));
        EXPECT_THAT(context.Get<std::string>("std::string"), testing::StrEq("abcdef"));
        EXPECT_THAT(context.Get<std::uint32_t>("std::uint32_t"), testing::Eq(10));
    }
}
