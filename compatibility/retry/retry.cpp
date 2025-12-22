#include "cucumber_cpp/CucumberCpp.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

GIVEN(R"(a step that always passes)")
{
    // no-op
}

namespace
{
    auto secondTimePass = 0;
    auto thirdTimePass = 0;
}

GIVEN(R"(a step that passes the second time)")
{
    secondTimePass++;
    EXPECT_THAT(secondTimePass, testing::Ge(2));
}

GIVEN(R"(a step that passes the third time)")
{
    // no-op
    thirdTimePass++;
    EXPECT_THAT(thirdTimePass, testing::Ge(3));
}

GIVEN(R"(a step that always fails)")
{
    // no-op
    FAIL();
}
