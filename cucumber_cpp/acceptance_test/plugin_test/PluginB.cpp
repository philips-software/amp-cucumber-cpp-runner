#include "cucumber_cpp/Steps.hpp"
#include "gtest/gtest.h"
#include <iostream>

HOOK_BEFORE_SCENARIO("@plugin_b_hook")
{
    std::cout << "PLUGIN_B_BEFORE_SCENARIO\n";
    context.InsertAt("plugin_b_hook_ran", true);
}

HOOK_AFTER_SCENARIO("@plugin_b_hook")
{
    std::cout << "PLUGIN_B_AFTER_SCENARIO\n";
}

THEN("plugin B can read the static context")
{
    ASSERT_TRUE(context.Get<bool>("static_step_ran"));
    context.InsertAt("plugin_b_wrote", true);
}

THEN("plugin B hook was executed")
{
    ASSERT_TRUE(context.Get<bool>("plugin_b_hook_ran"));
}
