#include "cucumber_cpp/library/Hooks.hpp"
#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include <gtest/gtest.h>
#include <string>

namespace cucumber_cpp::library::engine
{
    HOOK_BEFORE_ALL()
    {
        context.InsertAt("hookBeforeAll", std::string{ "hookBeforeAll" });
    }

    HOOK_AFTER_ALL()
    {
        context.InsertAt("hookAfterAll", std::string{ "hookAfterAll" });
    }

    HOOK_BEFORE_FEATURE()
    {
        context.InsertAt("hookBeforeFeature", std::string{ "hookBeforeFeature" });
    }

    HOOK_AFTER_FEATURE()
    {
        context.InsertAt("hookAfterFeature", std::string{ "hookAfterFeature" });
    }

    HOOK_BEFORE_SCENARIO()
    {
        context.InsertAt("hookBeforeScenario", std::string{ "hookBeforeScenario" });
    }

    HOOK_AFTER_SCENARIO()
    {
        context.InsertAt("hookAfterScenario", std::string{ "hookAfterScenario" });
    }

    HOOK_BEFORE_STEP()
    {
        context.InsertAt("hookBeforeStep", std::string{ "hookBeforeStep" });
    }

    HOOK_AFTER_STEP()
    {
        context.InsertAt("hookAfterStep", std::string{ "hookAfterStep" });
    }

    struct TestHookExecutor : testing::Test
    {
        test_helper::ContextManagerInstance contextManagerInstance;
        HookExecutorImpl hookExecutor{ contextManagerInstance };
    };

    TEST_F(TestHookExecutor, Construct)
    {
    }

    TEST_F(TestHookExecutor, ExecuteBeforeAll)
    {
        hookExecutor.BeforeAll();
        EXPECT_TRUE(contextManagerInstance.ProgramContext().Contains("hookBeforeAll"));
    }

    TEST_F(TestHookExecutor, ExecuteAfterAll)
    {
        hookExecutor.AfterAll();
        EXPECT_TRUE(contextManagerInstance.ProgramContext().Contains("hookAfterAll"));
    }

    TEST_F(TestHookExecutor, ExecuteBeforeFeature)
    {
        hookExecutor.BeforeFeature();
        EXPECT_TRUE(contextManagerInstance.FeatureContext().Contains("hookBeforeFeature"));
    }

    TEST_F(TestHookExecutor, ExecuteAfterFeature)
    {
        hookExecutor.AfterFeature();
        EXPECT_TRUE(contextManagerInstance.FeatureContext().Contains("hookAfterFeature"));
    }

    TEST_F(TestHookExecutor, ExecuteBeforeScenario)
    {
        hookExecutor.BeforeScenario();
        EXPECT_TRUE(contextManagerInstance.ScenarioContext().Contains("hookBeforeScenario"));
    }

    TEST_F(TestHookExecutor, ExecuteAfterScenario)
    {
        hookExecutor.AfterScenario();
        EXPECT_TRUE(contextManagerInstance.ScenarioContext().Contains("hookAfterScenario"));
    }

    TEST_F(TestHookExecutor, ExecuteBeforeStep)
    {
        hookExecutor.BeforeStep();
        EXPECT_TRUE(contextManagerInstance.StepContext().Contains("hookBeforeStep"));
    }

    TEST_F(TestHookExecutor, ExecuteAfterStep)
    {
        hookExecutor.AfterStep();
        EXPECT_TRUE(contextManagerInstance.StepContext().Contains("hookAfterStep"));
    }
}
