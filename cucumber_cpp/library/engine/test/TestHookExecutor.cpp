#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include <gtest/gtest.h>

namespace cucumber_cpp::library::engine
{
    struct TestHookExecutor : testing::Test
    {
        test_helper::ContextManagerInstance contextManagerInstance;
        HookExecutorImpl hookExecutor{ contextManagerInstance };
    };

    TEST_F(TestHookExecutor, Construct)
    {
    }

    TEST_F(TestHookExecutor, ExecuteProgramHooks)
    {
        ASSERT_FALSE(contextManagerInstance.ProgramContext().Contains("hookBeforeAll"));
        ASSERT_FALSE(contextManagerInstance.ProgramContext().Contains("hookAfterAll"));

        {
            auto scope = hookExecutor.BeforeAll();
            EXPECT_TRUE(contextManagerInstance.ProgramContext().Contains("hookBeforeAll"));
        }
        EXPECT_TRUE(contextManagerInstance.ProgramContext().Contains("hookAfterAll"));
    }

    TEST_F(TestHookExecutor, ExecuteBeforeFeature)
    {
        ASSERT_FALSE(contextManagerInstance.FeatureContext().Contains("hookBeforeFeature"));
        ASSERT_FALSE(contextManagerInstance.FeatureContext().Contains("hookAfterFeature"));

        {
            auto scope = hookExecutor.FeatureStart();
            EXPECT_TRUE(contextManagerInstance.FeatureContext().Contains("hookBeforeFeature"));
        }
        EXPECT_TRUE(contextManagerInstance.FeatureContext().Contains("hookAfterFeature"));
    }

    TEST_F(TestHookExecutor, ExecuteBeforeScenario)
    {
        ASSERT_FALSE(contextManagerInstance.ScenarioContext().Contains("hookBeforeScenario"));
        ASSERT_FALSE(contextManagerInstance.ScenarioContext().Contains("hookAfterScenario"));

        {
            auto scope = hookExecutor.ScenarioStart();
            EXPECT_TRUE(contextManagerInstance.ScenarioContext().Contains("hookBeforeScenario"));
        }
        EXPECT_TRUE(contextManagerInstance.ScenarioContext().Contains("hookAfterScenario"));
    }

    TEST_F(TestHookExecutor, ExecuteBeforeStep)
    {
        ASSERT_FALSE(contextManagerInstance.StepContext().Contains("hookBeforeStep"));
        ASSERT_FALSE(contextManagerInstance.StepContext().Contains("hookAfterStep"));

        {
            auto scope = hookExecutor.StepStart();
            EXPECT_TRUE(contextManagerInstance.StepContext().Contains("hookBeforeStep"));
        }
        EXPECT_TRUE(contextManagerInstance.StepContext().Contains("hookAfterStep"));
    }
}
