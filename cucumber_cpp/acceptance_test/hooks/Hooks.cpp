#include "cucumber_cpp/CucumberCpp.hpp"
#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

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

HOOK_BEFORE_SCENARIO("@scenariohook and @bats")
{
    std::cout << "HOOK_BEFORE_SCENARIO\n";
}

HOOK_AFTER_SCENARIO("@scenariohook and @bats")
{
    std::cout << "HOOK_AFTER_SCENARIO\n";
}

HOOK_BEFORE_STEP("@stephook and @bats")
{
    std::cout << "HOOK_BEFORE_STEP\n";
}

HOOK_AFTER_STEP("@stephook and @bats")
{
    std::cout << "HOOK_AFTER_STEP\n";
}

HOOK_BEFORE_SCENARIO("@fail_scenariohook_before")
{
    FAIL();
}

HOOK_AFTER_SCENARIO("@fail_scenariohook_after")
{
    FAIL();
}

HOOK_BEFORE_SCENARIO("@throw_scenariohook")
{
    throw std::string{ "error" };
}

HOOK_BEFORE_SCENARIO()
{
    if (context.Contains("--failprogramhook") && context.Get<bool>("--failprogramhook"))
        std::cout << "should not be executed\n";
}
