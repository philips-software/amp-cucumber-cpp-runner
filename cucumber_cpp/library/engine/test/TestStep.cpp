#include "cucumber/messages/pickle_step_argument.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>
#include <string>

namespace cucumber_cpp::library::engine
{
    struct StepMock : Step
    {
        using Step::Step;

        MOCK_METHOD(void, SetUp, (), (override));
        MOCK_METHOD(void, TearDown, (), (override));

        using Step::Pending;
        using Step::Skipped;

        using Step::Given;
        using Step::Then;
        using Step::When;

        using Step::context;
        using Step::dataTable;
        using Step::docString;
    };

    struct TestStep : testing::Test
    {
        util::Broadcaster broadcaster;
        std::shared_ptr<ContextStorageFactory> contextStorageFactory{ std::make_shared<ContextStorageFactoryImpl>() };
        Context context{ contextStorageFactory };
        engine::StepOrHookStarted stepOrHookStarted;
        cucumber::messages::pickle_step_argument pickleStepArgument;

        StepMock step{
            broadcaster,
            context,
            stepOrHookStarted,
            pickleStepArgument.data_table,
            pickleStepArgument.doc_string
        };
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
        ASSERT_THAT(context.Contains("top level value"), testing::Eq(false));

        step.context.InsertAt<std::string>("top level value", "value");

        ASSERT_THAT(context.Contains("top level value"), testing::Eq(true));
    }

    // TEST_F(TestStep, ProvidesAccessToTable)
    // {
    //     ASSERT_THAT(step.table[0][0].As<std::string>(), testing::Eq(table[0][0].As<std::string>()));
    //     ASSERT_THAT(step.table[1][1].As<std::string>(), testing::Eq(table[1][1].As<std::string>()));
    // }

    TEST_F(TestStep, ThrowsStepPendingExceptionOnPending)
    {
        ASSERT_THROW(step.Pending("message"), StepPending);
    }
}
