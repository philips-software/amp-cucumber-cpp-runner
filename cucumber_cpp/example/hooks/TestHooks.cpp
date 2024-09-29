#include "cucumber_cpp/library/Hooks.hpp"
#include <iostream>

HOOK_BEFORE_ALL()
{
    std::cout << "HOOK_BEFORE_ALL\n";
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
