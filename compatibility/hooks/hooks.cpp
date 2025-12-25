#include "cucumber_cpp/CucumberCpp.hpp"
#include <gmock/gmock.h>

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
    ASSERT_THAT(true, false);
}

HOOK_AFTER_SCENARIO()
{
    // no-op
}
