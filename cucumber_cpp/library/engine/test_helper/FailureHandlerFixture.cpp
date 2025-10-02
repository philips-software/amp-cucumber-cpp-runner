#include "cucumber_cpp/library/engine/test_helper/FailureHandlerFixture.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "gtest/gtest.h"

namespace cucumber_cpp::library::engine::test_helper
{
    FailureHandlerFixture::FailureHandlerFixture(ContextManager& contextManager)
        : contextManager{ contextManager }
    {
        auto& listeners = testing::UnitTest::GetInstance()->listeners();
        listeners.Append(&googleTestEventListener);
    }

    FailureHandlerFixture::~FailureHandlerFixture()
    {
        auto& listeners = testing::UnitTest::GetInstance()->listeners();
        listeners.Release(&googleTestEventListener);
    }
}
