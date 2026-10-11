#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/Steps.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <iostream>

HOOK_BEFORE_ALL()
{
    std::cout << "HOOK_BEFORE_ALL\n";

    if (context.Contains("--failprogramhook") && context.Get<bool>("--failprogramhook"))
        FAIL();
}

HOOK_AFTER_ALL()
{
    std::cout << "HOOK_AFTER_ALL\n";
}

HOOK_BEFORE_SCENARIO("@smoke and @result:OK", "fail if --failprogramhook is set")
{
    if (context.Contains("--failprogramhook") && context.Get<bool>("--failprogramhook"))
        std::cout << "should not be executed\n";
}
