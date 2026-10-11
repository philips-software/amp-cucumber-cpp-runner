Feature: Hooks

    Background:
        Given a file named "hooks.feature" with:
            """
@bats
Feature: Test scenario and step hook bindings

    @program_hooks
    Scenario: Run program hooks
        Given a given step

    @fail_scenariohook_before
    Scenario: Run failing Scenario hooks before
        Given a given step

    @fail_scenariohook_after
    Scenario: Run failing Scenario hooks after
        Given a given step

    @throw_scenariohook
    Scenario: Run throwing Scenario hooks
        Given a given step

    Rule: These scenarios have a background

        Background:
            Given a background step

        @scenariohook @stephook
        Scenario: Run Scenario and Step hooks
            Given a given step
            When a when step
            Then a then step

        @scenariohook
        Scenario: Run only Scenario hooks
            Given a given step
            When a when step
            Then a then step

        @stephook
        Scenario: Run only Step hooks
            Given a given step
            When a when step
            Then a then step
                """
            And a library named "hooks" with:
                """
                #include "cucumber_cpp/Steps.hpp"
                #include "cucumber_cpp/library/util/ScenarioInfo.hpp"
                #include "gmock/gmock.h"
                #include "gtest/gtest.h"
                #include <iostream>
                #include <string>

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

                HOOK_BEFORE_SCENARIO("@fail_scenariohook_before", "will fail before scenario")
                {
                FAIL();
                }

                HOOK_AFTER_SCENARIO("@fail_scenariohook_after")
                {
                FAIL();
                }

                HOOK_BEFORE_SCENARIO("@throw_scenariohook")
                {
                throw std::string{ "error" };
                }

                HOOK_BEFORE_SCENARIO("@expose_scenario_info")
                {
                const auto& scenarioInfo = ScenarioInfo();
                if (scenarioInfo.tags.contains("@store_scenario_info"))
                context.InsertAt("ScenarioInfoHook", scenarioInfo);
                }

                HOOK_BEFORE_STEP("@expose_scenario_info")
                {
                const auto& scenarioInfo = ScenarioInfo();
                if (scenarioInfo.tags.contains("@store_scenario_info"))
                context.InsertAt("StepHookInfo", scenarioInfo);
                }

                THEN("the {string} has the name {string} or {string}", (const std::string& hookType, const std::string& expectedName1, const std::string& expectedName2))
                {
                const auto& scenarioInfo = context.Get<cucumber_cpp::library::util::ScenarioInfo>(hookType);
                ASSERT_THAT(scenarioInfo.name, testing::AnyOf(testing::StrEq(expectedName1), testing::StrEq(expectedName2)));
                }

                THEN("the {string} has the name {string}", (const std::string& hookType, const std::string& expectedName))
                {
                const auto& scenarioInfo = context.Get<cucumber_cpp::library::util::ScenarioInfo>(hookType);
                ASSERT_THAT(scenarioInfo.name, testing::StrEq(expectedName));
                }

                THEN("the {string} has the tag {string} or {string}", (const std::string& hookType, const std::string& expectedTag1, const std::string& expectedTag2))
                {
                const auto& scenarioInfo = context.Get<cucumber_cpp::library::util::ScenarioInfo>(hookType);
                ASSERT_THAT(scenarioInfo.tags, testing::AnyOf(testing::Contains(expectedTag1), testing::Contains(expectedTag2)));
                }

                THEN("the {string} has the tags {string} and {string}", (const std::string& hookType, const std::string& expectedTag1, const std::string& expectedTag2))
                {
                const auto& scenarioInfo = context.Get<cucumber_cpp::library::util::ScenarioInfo>(hookType);
                ASSERT_THAT(scenarioInfo.tags, testing::AllOf(testing::Contains(expectedTag1), testing::Contains(expectedTag2)));
                }

                THEN("the {string} is not available", (const std::string& hookType))
                {
                ASSERT_THAT(context.Contains(hookType), testing::IsFalse());
                }

                GIVEN("a background step")
                {
                }

                GIVEN("a given step")
                {
                }

                WHEN("a when step")
                {
                }

                THEN("a then step")
                {
                }
                """

        Scenario: Program hooks run before and after all scenarios
            When I run cucumber-cpp-runner with "hooks" and arguments:
                | --format                 |
                | summary                  |
                | --tags                   |
                | @bats and @program_hooks |
                | --                       |
                | .                        |
            Then it passes
            And the output contains "HOOK_BEFORE_ALL"
            And the output contains "HOOK_AFTER_ALL"

        Scenario: Scenario hooks run only for matching scenarios
            When I run cucumber-cpp-runner with "hooks" and arguments:
                | --format                                  |
                | summary                                   |
                | --tags                                    |
                | @bats and @scenariohook and not @stephook |
                | --                                        |
                | .                                         |
            Then it passes
            And the output contains "HOOK_BEFORE_SCENARIO"
            And the output contains "HOOK_AFTER_SCENARIO"
            And the output does not contain "HOOK_BEFORE_STEP"
            And the output does not contain "HOOK_AFTER_STEP"

        Scenario: Step hooks run only for matching scenarios
            When I run cucumber-cpp-runner with "hooks" and arguments:
                | --format                                  |
                | summary                                   |
                | --tags                                    |
                | @bats and @stephook and not @scenariohook |
                | --                                        |
                | .                                         |
            Then it passes
            And the output does not contain "HOOK_BEFORE_SCENARIO"
            And the output does not contain "HOOK_AFTER_SCENARIO"
            And the output contains "HOOK_BEFORE_STEP"
            And the output contains "HOOK_AFTER_STEP"

        Scenario: Scenario and step hooks run together
            When I run cucumber-cpp-runner with "hooks" and arguments:
                | --format                               |
                | summary                                |
                | --tags                                 |
                | @bats and (@scenariohook or @stephook) |
                | --                                     |
                | .                                      |
            Then it passes
            And the output contains "HOOK_BEFORE_SCENARIO"
            And the output contains "HOOK_AFTER_SCENARIO"
            And the output contains "HOOK_BEFORE_STEP"
            And the output contains "HOOK_AFTER_STEP"

        Scenario: A failing named before scenario hook fails the run
            When I run cucumber-cpp-runner with "hooks" and arguments:
                | --format                         |
                | summary                          |
                | --format-options                 |
                | { "summary": {"theme":"plain"} } |
                | --tags                           |
                | @fail_scenariohook_before        |
                | --                               |
                | .                                |
            Then it fails
            And the output contains "Failed scenarios:"
            And the output contains "Before(will fail before scenario) #"

        Scenario: A failing after scenario hook fails the run
            When I run cucumber-cpp-runner with "hooks" and arguments:
                | --format                         |
                | summary                          |
                | --format-options                 |
                | { "summary": {"theme":"plain"} } |
                | --tags                           |
                | @fail_scenariohook_after         |
                | --                               |
                | .                                |
            Then it fails
            And the output contains "Failed scenarios:"
            And the output contains "After #"

        Scenario: A throwing before scenario hook fails the run
            When I run cucumber-cpp-runner with "hooks" and arguments:
                | --format                         |
                | summary                          |
                | --format-options                 |
                | { "summary": {"theme":"plain"} } |
                | --tags                           |
                | @throw_scenariohook              |
                | --                               |
                | .                                |
            Then it fails
            And the output contains "Failed scenarios:"
            And the output contains "Before #"

        Scenario: Scenario and step hooks can access the scenario information
            Given a file named "scenario_info.feature" with:
                """
    @expose_scenario_info
    Feature: Scenario begin and end hooks can expose scenario information
        @store_scenario_info
    Rule: Scenario begin and end hooks store the scenario information
        Background:
            Then the "ScenarioInfoHook" has the name "Scenario with @tag-a" or "Scenario with @tag-b"
            And the "ScenarioInfoHook" has the tag "@tag-a" or "@tag-b"
            And the "StepHookInfo" has the name "Scenario with @tag-a" or "Scenario with @tag-b"
            And the "StepHookInfo" has the tag "@tag-a" or "@tag-b"

        Scenario Outline: Scenario with <tag>
            Then the "<scope>" has the name "Scenario with <tag>"
            And the "<scope>" has the tags "@expose_scenario_info" and "<tag>"

            @tag-a
            Examples:
                | scope            | tag    |
                | ScenarioInfoHook | @tag-a |
                | StepHookInfo     | @tag-a |

            @tag-b
            Examples:
                | scope            | tag    |
                | ScenarioInfoHook | @tag-b |
                | StepHookInfo     | @tag-b |

        @no_store_scenario_info
    Rule: scenario begin and end hooks do not store the scenario information
        Scenario: Scenario information is available
            Then the "ScenarioInfoHook" is not available
            And the "StepHookInfo" is not available
                """
            When I run cucumber-cpp-runner with "hooks" and arguments:
                | --format              |
                | summary               |
                | --tags                |
                | @expose_scenario_info |
                | --                    |
                | .                     |
            Then it passes
            And the output contains "5 scenarios"
