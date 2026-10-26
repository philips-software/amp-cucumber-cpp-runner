Feature: Custom parameters based on a map of values

    Background:
        Given a library named "steps" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include "gmock/gmock.h"
            #include <cstdint>

            enum class State
            {
            connected = 10,
            disconnected = 20,
            standby = 30,
            };

            PARAMETER_MAP(State, ("state", true), ({ "connected", State::connected }, { "disconnected", State::disconnected }, { "standby", State::standby }));

            PARAMETER_MAP(int, ("number", false), ({ "on", 100 }, { "one", 1 }, { "c++", 3 }, { "a.b", 4 }));

            GIVEN("the device is {state}", (State state))
            {
            context.InsertAt("state", state);
            }

            THEN("the stored state is {state}", (State expected))
            {
            EXPECT_THAT(context.Get<State>("state"), testing::Eq(expected));
            }

            THEN("the stored state has value {int}", (std::int32_t expected))
            {
            EXPECT_THAT(static_cast<std::int32_t>(context.Get<State>("state")), testing::Eq(expected));
            }

            THEN("the number {number} equals {int}", (int number, std::int32_t expected))
            {
            EXPECT_THAT(number, testing::Eq(expected));
            }
            """

    Scenario: Keys are converted to their mapped values
        Given a file named "parameter_map.feature" with:
            """
Feature: Parameter map
    Scenario Outline: Convert <key>
        Given the device is <key>
        Then the stored state is <key>
        And the stored state has value <value>

        Examples:
            | key          | value |
            | connected    | 10    |
            | disconnected | 20    |
            | standby      | 30    |
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                         |
            | summary                          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it passes
        And the output contains "3 scenarios"
        And the output contains "3 passed"

    Scenario: A wrongly mapped value fails the step
        Given a file named "parameter_map.feature" with:
            """
Feature: Parameter map
    Scenario: Wrong value
        Given the device is standby
        Then the stored state has value 10
            """
        When I run cucumber-cpp-runner with "steps"
        Then it fails

    Scenario: A value that is not a key results in an undefined step
        Given a file named "parameter_map.feature" with:
            """
Feature: Parameter map
    Scenario: Unknown key
        Given the device is shutdown
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                         |
            | summary                          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it fails
        And the output contains "Undefined scenarios:"
        And the output contains "Given the device is shutdown"

    Scenario: Keys are matched case-sensitively
        Given a file named "parameter_map.feature" with:
            """
Feature: Parameter map
    Scenario: Different case
        Given the device is Connected
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                         |
            | summary                          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it fails
        And the output contains "Undefined scenarios:"
        And the output contains "Given the device is Connected"

    Scenario: Keys that are prefixes of other keys or contain regex characters are matched literally
        Given a file named "parameter_map.feature" with:
            """
Feature: Parameter map
    Scenario: Prefixes and special characters
        Then the number on equals 100
        And the number one equals 1
        And the number c++ equals 3
        And the number a.b equals 4
            """
        When I run cucumber-cpp-runner with "steps"
        Then it passes

    Scenario: Regex characters in keys are not interpreted
        Given a file named "parameter_map.feature" with:
            """
Feature: Parameter map
    Scenario: Dot is not a wildcard
        Then the number axb equals 4
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                         |
            | summary                          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it fails
        And the output contains "Undefined scenarios:"
