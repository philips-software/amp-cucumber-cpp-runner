#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/HookScopes.hpp"
#include "cucumber_cpp/library/Hooks.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace cucumber_cpp
{
    HOOK_BEFORE_ALL()
    {
        context.EmplaceAt<std::string>("BeforeAll", "BeforeAll");
    }

    HOOK_AFTER_ALL()
    {
        context.EmplaceAt<std::string>("AfterAll", "AfterAll");
    }

    HOOK_BEFORE_FEATURE()
    {
        context.EmplaceAt<std::string>("BeforeFeature", "BeforeFeature");
    }

    HOOK_BEFORE_FEATURE("@tag1")
    {
        context.EmplaceAt<std::string>("BeforeFeatureCond", "BeforeFeature@Tag1");
    }

    HOOK_AFTER_FEATURE("@tag2")
    {
        context.EmplaceAt<std::string>("AfterFeatureCond", "AfterFeatureCond@Tag2");
    }

    HOOK_AFTER_FEATURE("@tag1 and @tag2")
    {
        context.EmplaceAt<std::string>("AfterFeatureCond2", "AfterFeatureCond@Tag1&Tag2");
    }

    HOOK_AFTER_FEATURE()
    {
        context.EmplaceAt<std::string>("AfterFeature", "AfterFeature");
    }

    HOOK_BEFORE_SCENARIO("@tag1 or @tag2")
    {
        context.EmplaceAt<std::string>("BeforeScenarioCond", "BeforeScenarioCond@Tag|Tag2");
    }

    HOOK_BEFORE_SCENARIO()
    {
        context.EmplaceAt<std::string>("BeforeScenario", "BeforeScenario");
    }

    HOOK_AFTER_SCENARIO()
    {
        context.EmplaceAt<std::string>("AfterScenario", "AfterScenario");
    }

    HOOK_BEFORE_STEP()
    {
        context.EmplaceAt<std::string>("BeforeStep", "BeforeStep");
    }

    HOOK_AFTER_STEP()
    {
        context.EmplaceAt<std::string>("AfterStep", "AfterStep");
    }

    struct TestHooks : testing::Test
    {
        HookRegistry& hookRegistry{ HookRegistry::Instance() };

        library::engine::test_helper::ContextManagerInstance contextManager;
    };

    TEST_F(TestHooks, UnconditionalStep)
    {
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeAll"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterAll"), testing::IsFalse());

        {
            BeforeAfterAllScope beforeAfterAllScope{ contextManager.CurrentContext() };

            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeAll"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterAll"), testing::IsFalse());
        }

        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeAll"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterAll"), testing::IsTrue());
    }

    TEST_F(TestHooks, FeatureScope)
    {
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsFalse());

        {
            BeforeAfterFeatureHookScope beforeAfterFeatureHookScope{ contextManager.CurrentContext() };

            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsFalse());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsFalse());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsFalse());
        }

        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsTrue());
    }

    TEST_F(TestHooks, SingleTag)
    {
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsFalse());

        {
            BeforeAfterFeatureHookScope beforeAfterFeatureHookScope{ contextManager.CurrentContext(), { "@tag1" } };

            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsTrue());
            ASSERT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsFalse());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsFalse());

            EXPECT_THAT(contextManager.CurrentContext().Get<std::string>("BeforeFeatureCond"), testing::StrEq("BeforeFeature@Tag1"));
        }

        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsTrue());

        EXPECT_THAT(contextManager.CurrentContext().Get<std::string>("BeforeFeatureCond"), testing::StrEq("BeforeFeature@Tag1"));
    }

    TEST_F(TestHooks, MultiTag)
    {
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond2"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsFalse());

        {
            BeforeAfterFeatureHookScope beforeAfterFeatureHookScope{ contextManager.CurrentContext(), { "@tag1", "@tag2" } };

            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsFalse());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond2"), testing::IsFalse());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsFalse());

            EXPECT_THAT(contextManager.CurrentContext().Get<std::string>("BeforeFeatureCond"), testing::StrEq("BeforeFeature@Tag1"));
        }

        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeature"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeFeatureCond"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeatureCond2"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterFeature"), testing::IsTrue());

        EXPECT_THAT(contextManager.CurrentContext().Get<std::string>("BeforeFeatureCond"), testing::StrEq("BeforeFeature@Tag1"));
        EXPECT_THAT(contextManager.CurrentContext().Get<std::string>("AfterFeatureCond"), testing::StrEq("AfterFeatureCond@Tag2"));
        EXPECT_THAT(contextManager.CurrentContext().Get<std::string>("AfterFeatureCond2"), testing::StrEq("AfterFeatureCond@Tag1&Tag2"));
    }

    TEST_F(TestHooks, ScenarioScope)
    {
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeScenarioCond"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeScenario"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterScenario"), testing::IsFalse());

        {
            BeforeAfterHookScope beforeAfterHookScope{ contextManager.CurrentContext(), { "@tag3", "@tag1" } };

            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeScenarioCond"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeScenario"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterScenario"), testing::IsFalse());
        }

        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeScenarioCond"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeScenario"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterScenario"), testing::IsTrue());
    }

    TEST_F(TestHooks, StepScope)
    {
        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeStep"), testing::IsFalse());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterStep"), testing::IsFalse());

        {
            BeforeAfterStepHookScope beforeAfterStepHookScope{ contextManager.CurrentContext(), { "@tag3", "@tag1" } };

            EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeStep"), testing::IsTrue());
            EXPECT_THAT(contextManager.CurrentContext().Contains("AfterStep"), testing::IsFalse());
        }

        EXPECT_THAT(contextManager.CurrentContext().Contains("BeforeStep"), testing::IsTrue());
        EXPECT_THAT(contextManager.CurrentContext().Contains("AfterStep"), testing::IsTrue());
    }
}
