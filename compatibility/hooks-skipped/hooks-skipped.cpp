#include "cucumber_cpp/CucumberCpp.hpp"
#include <gmock/gmock.h>

HOOK_BEFORE_SCENARIO()
{
    // no-op
}

HOOK_BEFORE_SCENARIO("@skip-before")
{
    Skipped();
}

HOOK_BEFORE_SCENARIO()
{
    // no-op
}

WHEN(R"(a normal step)")
{
    // no-op
}

WHEN(R"(a step that skips)")
{
    Skipped();
}

HOOK_AFTER_SCENARIO()
{
    // no-op
}

HOOK_AFTER_SCENARIO("@skip-after")
{
    Skipped();
}

HOOK_AFTER_SCENARIO()
{
    // no-op
}
