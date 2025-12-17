#include "cucumber_cpp/CucumberCpp.hpp"
#include <gmock/gmock.h>

HOOK_BEFORE_ALL()
{
    // no-op
}

HOOK_BEFORE_ALL()
{
    // no-op
}

WHEN(R"(a step passes)")
{
    // no-op
}

HOOK_AFTER_ALL()
{
    // no-op
}

HOOK_AFTER_ALL()
{
    ASSERT_THAT(true, false);
}

HOOK_AFTER_ALL()
{
    // no-op
}
