#include "cucumber_cpp/CucumberCpp.hpp"
#include "gtest/gtest.h"
#include <cstdint>
#include <gmock/gmock.h>

GIVEN(R"(there are {int} cucumbers)", (std::int32_t initialCount))
{
    context.Insert(initialCount);
}

WHEN(R"(I eat {int} cucumbers)", (std::int32_t eatCount))
{
    context.Get<std::int32_t>() -= eatCount;
}

THEN(R"(I should have {int} cucumbers)", (std::int32_t expectedCount))
{
    ASSERT_THAT(context.Get<std::int32_t>(), testing::Eq(expectedCount));
}
