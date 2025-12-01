
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

namespace cucumber_cpp::library::engine
{
    struct TestContextManager : testing::Test
    {
        std::shared_ptr<ContextStorageFactoryImpl> contextStorageFactory = std::make_shared<ContextStorageFactoryImpl>();
        cucumber_cpp::library::engine::ContextManager contextManager{ contextStorageFactory };

        cucumber_cpp::library::engine::FeatureInfo feature{ {}, {} };
        cucumber_cpp::library::engine::RuleInfo rule{ feature, {}, {}, {}, {}, {} };
        cucumber_cpp::library::engine::ScenarioInfo scenario{ {},  rule, {}, {}, {}, {}};
        cucumber_cpp::library::engine::StepInfo step{ scenario, {}, {}, {}, {}, {}, {} };
    };

    TEST_F(TestContextManager, Construct)
    {}

    TEST_F(TestContextManager, ProgramContext)
    {
        auto& programContext = contextManager.ProgramContext();
        EXPECT_THAT(programContext.ExecutionStatus(), testing::Eq(cucumber_cpp::library::engine::Result::passed));
    }

    TEST_F(TestContextManager, FeatureContextNotAvailableAfterConstruction)
    {
        ASSERT_THROW(contextManager.FeatureContext(), cucumber_cpp::library::engine::ContextNotAvailable);

        try
        {
            contextManager.FeatureContext();
            FAIL() << "Expected to throw ContextNotAvailable";
        }
        catch (const cucumber_cpp::library::engine::ContextNotAvailable& e)
        {
            EXPECT_THAT(e.what(), testing::StrEq(R"(FeatureContext not available)"));
        }
    }

    TEST_F(TestContextManager, RuleContextNotAvailableAfterConstruction)
    {
        ASSERT_THROW(contextManager.RuleContext(), cucumber_cpp::library::engine::ContextNotAvailable);

        try
        {
            contextManager.RuleContext();
            FAIL() << "Expected to throw ContextNotAvailable";
        }
        catch (const cucumber_cpp::library::engine::ContextNotAvailable& e)
        {
            EXPECT_THAT(e.what(), testing::StrEq(R"(RuleContext not available)"));
        }
    }

    TEST_F(TestContextManager, ScenarioContextNotAvailableAfterConstruction)
    {
        ASSERT_THROW(contextManager.ScenarioContext(), cucumber_cpp::library::engine::ContextNotAvailable);

        try
        {
            contextManager.ScenarioContext();
            FAIL() << "Expected to throw ContextNotAvailable";
        }
        catch (const cucumber_cpp::library::engine::ContextNotAvailable& e)
        {
            EXPECT_THAT(e.what(), testing::StrEq(R"(ScenarioContext not available)"));
        }
    }

    TEST_F(TestContextManager, StepContextNotAvailableAfterConstruction)
    {
        ASSERT_THROW(contextManager.StepContext(), cucumber_cpp::library::engine::ContextNotAvailable);

        try
        {
            contextManager.StepContext();
            FAIL() << "Expected to throw ContextNotAvailable";
        }
        catch (const cucumber_cpp::library::engine::ContextNotAvailable& e)
        {
            EXPECT_THAT(e.what(), testing::StrEq(R"(StepContext not available)"));
        }
    }

    TEST_F(TestContextManager, FeatureContextLifetimeManagement)
    {
        {
            auto scoped = contextManager.CreateFeatureContext(feature);
            ASSERT_NO_THROW(contextManager.FeatureContext());
        }
        ASSERT_ANY_THROW(contextManager.FeatureContext());
    }

    TEST_F(TestContextManager, RuleContextLifetimeManagement)
    {
        {
            auto scopedFeature = contextManager.CreateFeatureContext(feature);
            auto scopedRule = contextManager.CreateRuleContext(rule);

            ASSERT_NO_THROW(contextManager.RuleContext());
        }
        ASSERT_ANY_THROW(contextManager.RuleContext());
    }

    TEST_F(TestContextManager, ScenarioContextLifetimeManagement)
    {
        {
            auto scopedFeature = contextManager.CreateFeatureContext(feature);
            auto scopedRule = contextManager.CreateRuleContext(rule);
            auto scopedScenario = contextManager.CreateScenarioContext(scenario);

            ASSERT_NO_THROW(contextManager.ScenarioContext());
        }
        ASSERT_ANY_THROW(contextManager.ScenarioContext());
    }

    TEST_F(TestContextManager, StepContextLifetimeManagement)
    {
        {
            auto scopedFeature = contextManager.CreateFeatureContext(feature);
            auto scopedRule = contextManager.CreateRuleContext(rule);
            auto scopedScenario = contextManager.CreateScenarioContext(scenario);
            auto stepContextScope = contextManager.CreateStepContext(step);

            ASSERT_NO_THROW(contextManager.StepContext());
        }
        ASSERT_ANY_THROW(contextManager.StepContext());
    }
}
