#include "cucumber_cpp/CucumberCpp.hpp"
#include <gtest/gtest.h>

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

WHEN(R"(a step fails)")
{
    FAIL();
}

HOOK_AFTER_ALL()
{
    // no-op
}

HOOK_AFTER_ALL()
{
    // no-op
}
