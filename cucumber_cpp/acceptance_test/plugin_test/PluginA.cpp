#include "cucumber_cpp/Steps.hpp"
#include "gtest/gtest.h"
#include <string>

namespace
{
    struct Color
    {
        std::string name;
        int r;
        int g;
        int b;
    };
}

PARAMETER(Color, ("color", "(red|green|blue)", false), (const std::string& name))
{
    if (name == "red")
        return Color{ .name = "red", .r = 255, .g = 0, .b = 0 };
    if (name == "green")
        return Color{ .name = "green", .r = 0, .g = 255, .b = 0 };
    return Color{ .name = "blue", .r = 0, .g = 0, .b = 255 };
}

THEN("plugin A can read the static context")
{
    ASSERT_TRUE(context.Get<bool>("static_step_ran"));
    context.InsertAt("plugin_a_wrote", true);
}

WHEN("a {color} is selected", (const Color& color))
{
    context.InsertAt("selected_color", color.name);
}

THEN("the color name is {string}", (const std::string& expected))
{
    ASSERT_EQ(context.Get<std::string>("selected_color"), expected);
}
