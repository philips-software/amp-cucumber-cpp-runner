#include "cucumber-cpp/Hooks.hpp"
#include "cucumber-cpp-example/fixtures/Fixture.hpp"
#include "cucumber-cpp/Context.hpp"
#include <algorithm>
#include <string_view>

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

HOOK_BEFORE_STEP()
{
}

HOOK_AFTER_STEP()
{
}

HOOK_AFTER_SCENARIO()
{
}

HOOK_AFTER_ALL()
{
}
