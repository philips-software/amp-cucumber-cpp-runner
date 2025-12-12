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
}

HOOK_BEFORE_SCENARIO("@result:OK")
{
}

HOOK_BEFORE_SCENARIO()
{
}

HOOK_AFTER_SCENARIO()
{
}

HOOK_BEFORE_STEP()
{
}

HOOK_AFTER_STEP()
{
}
