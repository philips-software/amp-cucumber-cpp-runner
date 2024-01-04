#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/Steps.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cucumber_cpp
{
    GIVEN("This is a GIVEN step")
    {}

    WHEN("This is a WHEN step")
    {}

    THEN("This is a THEN step")
    {}

    STEP("This is a STEP step")
    {}

    STEP("This is a step with a ([0-9]+)s delay", (std::uint32_t delay))
    {
        context->InsertAt("std::uint32_t", delay);
    }

    STEP("Step with cucumber expression syntax {float} {string} {int}", (float fl, std::string str, std::uint32_t nr))
    {
        context->InsertAt("float", fl);
        context->InsertAt("std::string", str);
        context->InsertAt("std::uint32_t", nr);
    }

    struct TestSteps : testing::Test
    {
        StepRepository stepRepository;
    };

    TEST_F(TestSteps, Construct)
    {
    }

    TEST_F(TestSteps, RegisterThroughPreregistration)
    {
        EXPECT_THAT(stepRepository.Size(), testing::Eq(6));
        EXPECT_THAT(stepRepository.Size(StepType::given), testing::Eq(1));
        EXPECT_THAT(stepRepository.Size(StepType::when), testing::Eq(1));
        EXPECT_THAT(stepRepository.Size(StepType::then), testing::Eq(1));
        EXPECT_THAT(stepRepository.Size(StepType::any), testing::Eq(3));
    }

    TEST_F(TestSteps, GetGivenStep)
    {
        const auto match = stepRepository.Get(StepType::given, "This is a GIVEN step");

        EXPECT_THAT(match.step->GetText(), testing::StrEq("This is a GIVEN step"));
        EXPECT_THAT(match.step->GetType(), testing::Eq(StepType::given));
    }

    TEST_F(TestSteps, GetWhenStep)
    {
        const auto match = stepRepository.Get(StepType::when, "This is a WHEN step");

        EXPECT_THAT(match.step->GetText(), testing::StrEq("This is a WHEN step"));
        EXPECT_THAT(match.step->GetType(), testing::Eq(StepType::when));
    }

    TEST_F(TestSteps, GetThenStep)
    {
        const auto match = stepRepository.Get(StepType::then, "This is a THEN step");

        EXPECT_THAT(match.step->GetText(), testing::StrEq("This is a THEN step"));
        EXPECT_THAT(match.step->GetType(), testing::Eq(StepType::then));
    }

    TEST_F(TestSteps, GetAnyStep)
    {
        const auto match = stepRepository.Get(StepType::given, "This is a STEP step");

        EXPECT_THAT(match.step->GetText(), testing::StrEq("This is a STEP step"));
        EXPECT_THAT(match.step->GetType(), testing::Eq(StepType::any));
    }

    TEST_F(TestSteps, GetStepWithMatches)
    {
        const auto match = stepRepository.Get(StepType::when, "This is a step with a 10s delay");

        EXPECT_THAT(match.step->GetText(), testing::StrEq("This is a step with a ([0-9]+)s delay"));
        EXPECT_THAT(match.step->GetType(), testing::Eq(StepType::any));
        EXPECT_THAT(match.regexMatch->Matches().size(), testing::Eq(1));
        EXPECT_THAT(match.regexMatch->Matches()[0], testing::StrEq("10"));
    }

    TEST_F(TestSteps, GetInvalidStep)
    {
        EXPECT_THROW(stepRepository.Get(StepType::when, "This step does not exist"), StepNotFoundException);
    }

    TEST_F(TestSteps, InvokeTestWithCucumberExpressions)
    {
        const auto match = stepRepository.Get(StepType::when, R"(Step with cucumber expression syntax 1.5 """abcdef"" 10)");

        auto contextStorage{ std::make_shared<ContextStorageFactoryImpl>() };
        Context context{ contextStorage };

        match.step->Run(context, match.regexMatch->Matches(), {});

        EXPECT_THAT(context.Contains("float"), testing::IsTrue());
        EXPECT_THAT(context.Contains("std::string"), testing::IsTrue());
        EXPECT_THAT(context.Contains("std::uint32_t"), testing::IsTrue());

        EXPECT_THAT(context.Get<float>("float"), testing::FloatEq(1.5));
        EXPECT_THAT(context.Get<std::string>("std::string"), testing::StrEq("abcdef"));
        EXPECT_THAT(context.Get<std::uint32_t>("std::uint32_t"), testing::Eq(10));
    }
}
