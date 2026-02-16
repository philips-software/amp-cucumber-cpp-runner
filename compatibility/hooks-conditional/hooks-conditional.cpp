#include "cucumber_cpp/CucumberCpp.hpp"
#include <gtest/gtest.h>

HOOK_BEFORE_SCENARIO("@passing-hook")
{
    // no-op
}

HOOK_BEFORE_SCENARIO("@fail-before")
{
    FAIL();
}

WHEN(R"(a step passes)")
{
    // no-op
}

HOOK_AFTER_SCENARIO("@fail-after")
{
    FAIL();
}

HOOK_AFTER_SCENARIO("@passing-hook")
{
    // no-op
}
