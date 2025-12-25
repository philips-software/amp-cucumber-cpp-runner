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

HOOK_BEFORE_ALL(.name = "Initialize something")
{
    /* no body, example only */
}

HOOK_BEFORE_SCENARIO(.name = "explicit name only")
{
    /* no body, example only */
}

HOOK_BEFORE_SCENARIO("@dingus", "name", 10)
{
    /* no body, example only */
}

HOOK_BEFORE_SCENARIO("@result:OK")
{
    /* no body, example only */
}

HOOK_BEFORE_SCENARIO()
{
    /* no body, example only */
}

HOOK_AFTER_SCENARIO()
{
    /* no body, example only */
}

HOOK_BEFORE_STEP()
{
    /* no body, example only */
}

HOOK_AFTER_STEP()
{
    /* no body, example only */
}
