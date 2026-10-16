Feature: Plugin loading

    Scenario: Load all plugins from a directory with shared context, custom parameters and hooks
        Given a file named "plugins.feature" with:
            """
Feature: Plugins loaded together
    @plugin_a @plugin_b_hook
    Scenario: Both plugins share the scenario context
        Given a shared plugin context
        Then plugin A can read the shared context
        And plugin B can read plugin A context
        When a green is selected
        Then the color name is "green"
        And plugin A can read plugin B context
        And plugin B hook was executed
            """
        And a library named "plugin_a" with:
            """
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

            THEN("plugin A can read the shared context")
            {
            ASSERT_TRUE(context.Get<bool>("shared_plugin_context"));
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

            THEN("plugin A can read plugin B context")
            {
            ASSERT_TRUE(context.Get<bool>("plugin_b_wrote"));
            }
            """
        And a library named "plugin_b" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include "gtest/gtest.h"
            #include <iostream>

            HOOK_BEFORE_SCENARIO("@plugin_b_hook")
            {
            std::cout << "PLUGIN_B_BEFORE_SCENARIO\n";
            context.InsertAt("plugin_b_hook_ran", true);
            }

            HOOK_AFTER_SCENARIO("@plugin_b_hook")
            {
            std::cout << "PLUGIN_B_AFTER_SCENARIO\n";
            }

            GIVEN("a shared plugin context")
            {
            context.InsertAt("shared_plugin_context", true);
            }

            THEN("plugin B can read plugin A context")
            {
            ASSERT_TRUE(context.Get<bool>("plugin_a_wrote"));
            context.InsertAt("plugin_b_wrote", true);
            }

            THEN("plugin B hook was executed")
            {
            ASSERT_TRUE(context.Get<bool>("plugin_b_hook_ran"));
            }
            """
        When I run cucumber-cpp-runner with arguments:
            | --load                           |
            | .                                |
            | --format                         |
            | summary                          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it passes
        And the output contains "PLUGIN_B_BEFORE_SCENARIO"
        And the output contains "PLUGIN_B_AFTER_SCENARIO"
        And the output contains "1 scenario (1 passed)"
        And the output contains "9 steps (9 passed)"
