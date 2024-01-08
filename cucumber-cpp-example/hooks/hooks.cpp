#include "cucumber-cpp/Hooks.hpp"
#include "cucumber-cpp-example/fixtures/Fixture.hpp"
#include "cucumber-cpp/Context.hpp"
#include <algorithm>
#include <string_view>

HOOK_BEFORE_ALL()
{
    const auto& args = context.Get<std::vector<std::string_view>>("args");

    if (std::ranges::find(args, "--st") != args.end())
    {
        context.Emplace<StBleFixture>();
    }
    else if (std::ranges::find(args, "--nordic") != args.end())
    {
        context.Emplace<NordicBleFixture>();
    }
    else
    {
        throw "error";
    }

    // spawn eventdispatcher
}

HOOK_BEFORE_SCENARIO()
{
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
