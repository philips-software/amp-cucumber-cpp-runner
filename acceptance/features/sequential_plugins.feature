Feature: Sequential plugin loading

    Scenario: A custom host loads plugins in successive application runs with static steps
        Given a file named "sequential_plugins.feature" with:
            """
Feature: Plugins share context with statically linked steps
    @plugin_a
    Scenario: Plugin A provides a custom parameter and shares static context
        Given a statically linked step
        Then plugin A can read the static context
        When a red is selected
        Then the color name is "red"
        And the static step can read plugin A context

    @plugin_a
    Scenario: Plugin A converts another color
        Given a statically linked step
        Then plugin A can read the static context
        When a blue is selected
        Then the color name is "blue"
        And the static step can read plugin A context

    @plugin_b_hook
    Scenario: Plugin B provides hooks and shares static context
        Given a statically linked step
        Then plugin B can read the static context
        And the static step can read plugin B context
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

            THEN("plugin B can read the static context")
            {
            ASSERT_TRUE(context.Get<bool>("static_step_ran"));
            context.InsertAt("plugin_b_wrote", true);
            }

            THEN("plugin B hook was executed")
            {
            ASSERT_TRUE(context.Get<bool>("plugin_b_hook_ran"));
            }
            """
        And an executable named "sequential_host" with:
            """
            #include "cucumber_cpp/CucumberCpp.hpp"
            #include "cucumber_cpp/Steps.hpp"
            #include "cucumber_cpp/library/plugin/DynamicLibrary.hpp"
            #include "gtest/gtest.h"
            #include <cstdlib>
            #include <iostream>
            #include <string>
            #include <vector>

            GIVEN("a statically linked step")
            {
            context.InsertAt("static_step_ran", true);
            }

            THEN("the static step can read plugin A context")
            {
            ASSERT_TRUE(context.Get<bool>("plugin_a_wrote"));
            }

            THEN("the static step can read plugin B context")
            {
            ASSERT_TRUE(context.Get<bool>("plugin_b_wrote"));
            }

            namespace
            {
            int RunApplication(const std::string& pluginName, const std::string& tags)
            {
            const auto pluginPath = pluginName + std::string{ cucumber_cpp::library::plugin::DynamicLibrary::PlatformExtension() };
            const std::vector<std::string> arguments{
                "sequential_host",
                "--load", pluginPath,
                "--format", "summary",
                "--format-options", R"({ "summary": {"theme":"plain"} })",
                "--tags", tags,
                "--", "sequential_plugins.feature"
            };

            std::vector<const char*> argv;
            argv.reserve(arguments.size());
            for (const auto& argument : arguments)
                argv.push_back(argument.c_str());

            cucumber_cpp::Application application{};
            return application.Run(static_cast<int>(argv.size()), argv.data());
            }
            }

            int main(int argc, char** argv)
            {
            if (argc != 3)
            {
                std::cerr << "Expected two plugin names\n";
                return EXIT_FAILURE;
            }

            if (RunApplication(argv[1], "@plugin_a and not @plugin_b_hook") != 0)
            {
                std::cerr << "FAILED: plugin A run\n";
                return EXIT_FAILURE;
            }
            std::cout << "PLUGIN_A_RUN_PASSED\n";

            if (RunApplication(argv[2], "@plugin_b_hook and not @plugin_a") != 0)
            {
                std::cerr << "FAILED: plugin B run\n";
                return EXIT_FAILURE;
            }
            std::cout << "PLUGIN_B_RUN_PASSED\n";

            return EXIT_SUCCESS;
            }
            """
        When I run the executable "sequential_host" with arguments:
            | plugin_a |
            | plugin_b |
        Then it passes
        And the output contains "PLUGIN_A_RUN_PASSED"
        And the output contains "PLUGIN_B_RUN_PASSED"
        And the output contains "PLUGIN_B_BEFORE_SCENARIO"
        And the output contains "PLUGIN_B_AFTER_SCENARIO"
        And the output contains "2 scenarios (2 passed)"
        And the output contains "1 scenario (1 passed)"
