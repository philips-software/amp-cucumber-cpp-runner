#include "cucumber_cpp/Hooks.hpp"
#include "cucumber_cpp_example/fixtures/Fixture.hpp"
#include "cucumber_cpp/Context.hpp"
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
