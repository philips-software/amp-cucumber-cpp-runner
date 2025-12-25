#include "cucumber_cpp/CucumberCpp.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <string>

GIVEN(R"(there are {int} cucumbers)", (std::int32_t cucumbers))
{
    context.InsertAt("cucumbers", cucumbers);
}

GIVEN(R"(there are {int} friends)", (std::int32_t friends))
{
    context.InsertAt("friends", friends);
}

WHEN(R"(I eat {int} cucumbers)", (std::int32_t eatCount))
{
    context.Get<std::int32_t>("cucumbers") -= eatCount;
}

THEN(R"(I should have {int} cucumbers)", (std::int32_t expectedCount))
{
    ASSERT_THAT(context.Get<std::int32_t>("cucumbers"), testing::Eq(expectedCount));
}

THEN(R"(each person can eat {int} cucumbers)", (std::int32_t expectedShare))
{
    const auto share = context.Get<std::int32_t>("cucumbers") / (1 + context.Get<std::int32_t>("friends"));
    ASSERT_THAT(share, testing::Eq(expectedShare));
}
