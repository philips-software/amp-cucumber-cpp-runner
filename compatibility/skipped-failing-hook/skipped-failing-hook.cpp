#include "cucumber_cpp/Steps.hpp"
#include <gtest/gtest.h>

GIVEN(R"(a step that skips)")
{
    Skipped();
}

HOOK_AFTER_SCENARIO()
{
    FAIL();
}
