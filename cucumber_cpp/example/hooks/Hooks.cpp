#include "cucumber_cpp/library/Hooks.hpp"
#include "cucumber_cpp/example/fixtures/Fixture.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include <iostream>
#include <string>

HOOK_BEFORE_ALL()
{
    if (context.Contains("--st") && context.Get<bool>("--st"))
        context.Emplace<StBleFixture>(context.Get<std::string>("--com"));
    else if (context.Contains("--nordic") && context.Get<bool>("--nordic"))
        context.Emplace<NordicBleFixture>();
}

HOOK_BEFORE_SCENARIO("@dingus")
{
    std::cout << "running only for dingus tests\n";
}

HOOK_BEFORE_SCENARIO()
{
    std::cout << "running before every test case\n";
}

HOOK_BEFORE_SCENARIO("@result:OK")
{
    std::cout << "running only for result:OK test cases\n";
}

HOOK_AFTER_SCENARIO()
{
    std::cout << "running after every test case\n";
}

HOOK_BEFORE_STEP()
{
    std::cout << "running before every test step\n";
}

HOOK_AFTER_STEP()
{
    std::cout << "running after every test step\n";
}
