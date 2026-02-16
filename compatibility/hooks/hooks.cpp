#include "cucumber_cpp/CucumberCpp.hpp"
#include <gtest/gtest.h>

HOOK_BEFORE_SCENARIO()
{
    // no-op
}

WHEN(R"(a step passes)")
{
    // no-op
}

WHEN(R"(a step fails)")
{
    FAIL();
}

HOOK_AFTER_SCENARIO()
{
    // no-op
}
