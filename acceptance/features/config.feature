Feature: Configuration file roundtrip

    Background:
        Given a file named "roundtrip.feature" with:
            """
Feature: Roundtrip feature
    Scenario: Selected scenario
        Given a given step

    Scenario: Other scenario
        Then an assertion is raised
            """
        And a library named "steps" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include "gmock/gmock.h"

            GIVEN("a given step")
            {
            }

            THEN("an assertion is raised")
            {
            ASSERT_THAT(false, testing::IsTrue());
            }
            """

    Scenario: A dumped configuration without scenario listing or name filters runs all scenarios
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --dump-config |
            | --format      |
            | summary       |
            | --            |
            | .             |
        Then it fails
        And the output contains "2 scenarios"
        When I run cucumber-cpp-runner with arguments:
            | --config      |
            | cucumber.toml |
        Then it fails
        And the output contains "2 scenarios"

    Scenario: A dumped configuration with scenario listing lists scenarios
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --list-scenarios |
            | --dump-config    |
            | --               |
            | .                |
        Then it passes
        And the output contains "Roundtrip feature.Selected scenario"
        And the output contains "Roundtrip feature.Other scenario"
        When I run cucumber-cpp-runner with arguments:
            | --config      |
            | cucumber.toml |
        Then it passes
        And the output contains "Roundtrip feature.Selected scenario"
        And the output contains "Roundtrip feature.Other scenario"

    Scenario: A dumped configuration with a name filter runs only matching scenarios
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --name              |
            | ^Selected scenario$ |
            | --dump-config       |
            | --format            |
            | summary             |
            | --                  |
            | .                   |
        Then it passes
        And the output contains "1 scenario"
        When I run cucumber-cpp-runner with arguments:
            | --config      |
            | cucumber.toml |
        Then it passes
        And the output contains "1 scenario"
        And the output does not contain "2 scenarios"
