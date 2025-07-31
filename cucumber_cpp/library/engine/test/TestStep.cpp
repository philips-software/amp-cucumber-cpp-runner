#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "cucumber_cpp/library/engine/test_helper/TestRunnerMock.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct StepMock : Step
    {
        using Step::Step;

        MOCK_METHOD(void, SetUp, (), (override));
        MOCK_METHOD(void, TearDown, (), (override));

        using Step::context;
        using Step::Given;
        using Step::Pending;
        using Step::table;
        using Step::Then;
        using Step::When;
    };

    struct TestStep : testing::Test
    {
        Table table{
            std::vector{ TableValue{ "header1" }, TableValue{ "header2}" } },
            std::vector{ TableValue{ "value1" }, TableValue{ "value2}" } }
        };

        const std::string docString = "multiline \n string";

        library::engine::test_helper::ContextManagerInstance contextManager;

        engine::test_helper::TestRunnerMock testRunnerMock;
        StepMock step{ contextManager.StepContext(), table, docString };
    };

    TEST_F(TestStep, StepProvidesAccessToSetUpFunction)
    {
        EXPECT_CALL(step, SetUp());

        static_cast<Step&>(step).SetUp();
    }

    TEST_F(TestStep, StepProvidesAccessToTearDownFunction)
    {
        EXPECT_CALL(step, TearDown());

        static_cast<Step&>(step).TearDown();
    }

    TEST_F(TestStep, ProvidesAccessToCurrentContext)
    {
        ASSERT_THAT(contextManager.StepContext().Contains("top level value"), testing::Eq(false));

        step.context.InsertAt<std::string>("top level value", "value");

        ASSERT_THAT(contextManager.StepContext().Contains("top level value"), testing::Eq(true));
    }

    TEST_F(TestStep, ProvidesAccessToTable)
    {
        ASSERT_THAT(step.table[0][0].As<std::string>(), testing::Eq(table[0][0].As<std::string>()));
        ASSERT_THAT(step.table[1][1].As<std::string>(), testing::Eq(table[1][1].As<std::string>()));
    }

    TEST_F(TestStep, ThrowsStepPendingExceptionOnPending)
    {
        ASSERT_THROW(step.Pending("message"), Step::StepPending);
    }
}
