#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include "cucumber_cpp/library/engine/test_helper/ContextManagerInstance.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
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

    /////////////////////////////////////////
    class FooTest : public testing::TestWithParam<std::string_view>
    {
        // You can implement all the usual fixture class members here.
        // To access the test parameter, call GetParam() from class
        // TestWithParam<T>.
    };

    // Or, when you want to add parameters to a pre-existing fixture class:
    class BaseTest : public testing::Test
    {
    };

    class BarTest : public BaseTest
        , public testing::WithParamInterface<std::string_view>
    {
    };

    TEST_P(FooTest, DoesBlah)
    {
        // Inside a test, access the test parameter with the GetParam() method
        // of the TestWithParam<T> class:
    }

    TEST_P(FooTest, HasBlahBlah)
    {
    }

    INSTANTIATE_TEST_SUITE_P(MeenyMinyMoe,
        FooTest,
        testing::Values("meeny", "miny", "moe"));
}
