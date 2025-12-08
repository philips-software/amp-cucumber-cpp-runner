// #include "cucumber_cpp/library/engine/FailureHandler.hpp"
// #include "cucumber_cpp/library/engine/HookExecutor.hpp"
// #include "cucumber_cpp/library/engine/Result.hpp"
// #include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
// #include "cucumber_cpp/library/engine/test_helper/FailureHandlerFixture.hpp"
// #include "cucumber_cpp/library/report/test_helper/ReportForwarderMock.hpp"
// #include <functional>
// #include <gmock/gmock.h>
// #include <gtest/gtest-spi.h>
// #include <gtest/gtest.h>
// #include <optional>
// #include <set>
// #include <string>
// #include <utility>

// namespace cucumber_cpp::library::engine
// {
//     namespace
//     {
//         std::function<void()> expectFatalStatement;
//     }

//     struct TestHookExecutor : testing::Test
//     {
//         std::optional<test_helper::ContextManagerInstance> contextManagerInstance{ std::in_place };
//         std::optional<HookExecutorImpl> hookExecutor{ *contextManagerInstance };

//         test_helper::FailureHandlerFixture failureHandlerFixture{ *contextManagerInstance };
//     };

//     TEST_F(TestHookExecutor, Construct)
//     {
//     }

//     TEST_F(TestHookExecutor, ExecuteProgramHooks)
//     {
//         ASSERT_FALSE(contextManagerInstance->ProgramContext().Contains("hookBeforeAll"));
//         ASSERT_FALSE(contextManagerInstance->ProgramContext().Contains("hookAfterAll"));

//         {
//             auto scope = hookExecutor->BeforeAll();
//             EXPECT_TRUE(contextManagerInstance->ProgramContext().Contains("hookBeforeAll"));
//         }
//         EXPECT_TRUE(contextManagerInstance->ProgramContext().Contains("hookAfterAll"));
//     }

//     TEST_F(TestHookExecutor, ExecuteBeforeFeature)
//     {
//         ASSERT_FALSE(contextManagerInstance->FeatureContext().Contains("hookBeforeFeature"));
//         ASSERT_FALSE(contextManagerInstance->FeatureContext().Contains("hookAfterFeature"));

//         {
//             auto scope = hookExecutor->FeatureStart();
//             EXPECT_TRUE(contextManagerInstance->FeatureContext().Contains("hookBeforeFeature"));
//         }
//         EXPECT_TRUE(contextManagerInstance->FeatureContext().Contains("hookAfterFeature"));
//     }

//     TEST_F(TestHookExecutor, ExecuteBeforeScenario)
//     {
//         ASSERT_FALSE(contextManagerInstance->ScenarioContext().Contains("hookBeforeScenario"));
//         ASSERT_FALSE(contextManagerInstance->ScenarioContext().Contains("hookAfterScenario"));

//         {
//             auto scope = hookExecutor->ScenarioStart();
//             EXPECT_TRUE(contextManagerInstance->ScenarioContext().Contains("hookBeforeScenario"));
//         }
//         EXPECT_TRUE(contextManagerInstance->ScenarioContext().Contains("hookAfterScenario"));
//     }

//     TEST_F(TestHookExecutor, ExecuteBeforeStep)
//     {
//         ASSERT_FALSE(contextManagerInstance->ScenarioContext().Contains("hookBeforeStep"));
//         ASSERT_FALSE(contextManagerInstance->ScenarioContext().Contains("hookAfterStep"));

//         {
//             auto scope = hookExecutor->StepStart();
//             EXPECT_TRUE(contextManagerInstance->ScenarioContext().Contains("hookBeforeStep"));
//         }
//         EXPECT_TRUE(contextManagerInstance->ScenarioContext().Contains("hookAfterStep"));
//     }

//     TEST_F(TestHookExecutor, BeforeHookError)
//     {
//         contextManagerInstance.emplace(std::set<std::string, std::less<>>{ "@errorbefore" });
//         hookExecutor.emplace(*contextManagerInstance);

//         report::test_helper::ReportForwarderMock reportHandler{ *contextManagerInstance };
//         TestAssertionHandlerImpl assertionHandler{ *contextManagerInstance, reportHandler };

//         expectFatalStatement = [this]
//         {
//             auto hook = hookExecutor->StepStart();
//         };

//         EXPECT_FATAL_FAILURE(expectFatalStatement(), "Expected: false");
//     }
// }
