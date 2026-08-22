#include "cucumber_cpp/Steps.hpp"
#include "gtest/gtest.h"
#include <string>

GIVEN("a statically linked step")
{
    context.InsertAt("static_step_ran", true);
}

THEN("the static step can read plugin A context")
{
    ASSERT_TRUE(context.Get<bool>("plugin_a_wrote"));
}

THEN("the static step can read plugin B context")
{
    ASSERT_TRUE(context.Get<bool>("plugin_b_wrote"));
}
