#include "cucumber_cpp/CucumberCpp.hpp"
#include <gmock/gmock.h>

HOOK_BEFORE_SCENARIO("@passing-hook")
{
    // no-op
}

HOOK_BEFORE_SCENARIO("@fail-before")
{
    ASSERT_THAT(true, false);
}

WHEN(R"(a step passes)")
{
    // no-op
}

HOOK_AFTER_SCENARIO("@fail-after")
{
    ASSERT_THAT(true, false);
}

HOOK_AFTER_SCENARIO("@passing-hook")
{
    // no-op
}
