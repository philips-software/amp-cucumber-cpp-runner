#include "cucumber/gherkin/id_generator.hpp"
#include "cucumber/messages/pickle_step_argument.hpp"
#include "cucumber/messages/test_step_started.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"
#include "cucumber_cpp/library/support/StepRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"
#include "cucumber_cpp/library/support/UndefinedParameters.hpp"
#include "cucumber_cpp/library/util/Broadcaster.hpp"
#include "cucumber_cpp/library/util/TransformDocString.hpp"
#include "cucumber_cpp/library/util/TransformTable.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <memory>
#include <string>

namespace cucumber_cpp::library::engine
{
    struct StepMock : StepBase
    {
        using StepBase::StepBase;

        MOCK_METHOD(void, SetUp, (), (override));
        MOCK_METHOD(void, TearDown, (), (override));

        using StepBase::Pending;
        using StepBase::Skipped;

        using StepBase::Step;

        using StepBase::context;
        using StepBase::dataTable;
        using StepBase::docString;
    };

    struct TestStep : testing::Test
    {
        util::Broadcaster broadcaster;
        std::shared_ptr<ContextStorageFactory> contextStorageFactory{ std::make_shared<ContextStorageFactoryImpl>() };
        Context context{ contextStorageFactory };
        engine::StepOrHookStarted stepOrHookStarted;
        cucumber::messages::pickle_step_argument pickleStepArgument;

        cucumber_expression::ParameterRegistry parameterRegistry{ cucumber_cpp::library::support::DefinitionRegistration::Instance().GetRegisteredParameters() };
        cucumber::gherkin::id_generator_ptr idGenerator = std::make_shared<cucumber::gherkin::id_generator>();
        support::UndefinedParameters undefinedParameters;
        support::StepRegistry stepRegistry{ parameterRegistry, undefinedParameters, idGenerator };
        support::HookRegistry hookRegistry{ idGenerator };

        support::SupportCodeLibrary supportCodeLibrary{
            .hookRegistry = hookRegistry,
            .stepRegistry = stepRegistry,
            .parameterRegistry = parameterRegistry,
            .undefinedParameters = undefinedParameters
        };
        runtime::NestedTestCaseRunner nestedTestCaseRunner{
            0,
            supportCodeLibrary,
            broadcaster,
            context,
            std::get<cucumber::messages::test_step_started>(stepOrHookStarted),
        };

        StepMock step{
            nestedTestCaseRunner,
            broadcaster,
            context,
            stepOrHookStarted,
            util::TransformTable(pickleStepArgument.data_table),
            util::TransformDocString(pickleStepArgument.doc_string),
        };
    };

    TEST_F(TestStep, StepProvidesAccessToSetUpFunction)
    {
        EXPECT_CALL(step, SetUp());

        static_cast<StepBase&>(step).SetUp();
    }

    TEST_F(TestStep, StepProvidesAccessToTearDownFunction)
    {
        EXPECT_CALL(step, TearDown());

        static_cast<StepBase&>(step).TearDown();
    }

    TEST_F(TestStep, ProvidesAccessToCurrentContext)
    {
        ASSERT_THAT(context.Contains("top level value"), testing::Eq(false));

        step.context.InsertAt<std::string>("top level value", "value");

        ASSERT_THAT(context.Contains("top level value"), testing::Eq(true));
    }

    TEST_F(TestStep, ProvidesAccessToTable)
    {
        ASSERT_THAT(step.dataTable.has_value(), testing::Eq(false));
    }

    TEST_F(TestStep, ThrowsStepPendingExceptionOnPending)
    {
        ASSERT_THROW(step.Pending("message"), StepPending);
    }
}
