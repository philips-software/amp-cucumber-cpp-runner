#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace cucumber_cpp
{
    struct StepMock : Step
    {
        using Step::Step;

        MOCK_METHOD(void, SetUp, (), (override));
        MOCK_METHOD(void, TearDown, (), (override));

        using Step::context;
        using Step::Pending;
        using Step::table;
    };

    struct TestStep : testing::Test
    {
        std::shared_ptr<ContextStorageFactoryImpl> contextStorageFactory{ std::make_shared<ContextStorageFactoryImpl>() };
        engine::ContextManager contextManager{ contextStorageFactory };

        Table table{
            std::vector{ TableValue{ "header1" }, TableValue{ "header2}" } },
            std::vector{ TableValue{ "value1" }, TableValue{ "value2}" } }
        };

        StepMock step{ contextManager.CurrentContext(), table };
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
        ASSERT_THAT(contextManager.CurrentContext().Contains("top level value"), testing::Eq(false));

        step.context.InsertAt<std::string>("top level value", "value");

        ASSERT_THAT(contextManager.CurrentContext().Contains("top level value"), testing::Eq(true));
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
