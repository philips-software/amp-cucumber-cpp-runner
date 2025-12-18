#include "cucumber_cpp/CucumberCpp.hpp"
#include <gmock/gmock.h>

HOOK_BEFORE_SCENARIO(.name = "A named before hook")
{
    // no-op
}

WHEN(R"(a step passes)")
{
    // no-op
}

HOOK_AFTER_SCENARIO(.name = "A named after hook")
{
    // no-op
}
