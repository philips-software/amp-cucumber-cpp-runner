Feature: Overall run result

    Background:
        Given a library named "steps" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include "gmock/gmock.h"

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

            THEN("an assertion is raised")
            {
            ASSERT_THAT(false, testing::IsTrue());
            }
            """

    Scenario: Undefined steps fail the run and are reported
        Given a file named "undefined.feature" with:
            """
Feature: Undefined steps
    Scenario: A scenario with undefined step
        Given a missing step
        Then a then step
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
        And the output contains "Given a missing step"

    Scenario: A later passing feature does not overwrite an earlier undefined result
        Given a file named "undefined_first.feature" with:
            """
Feature: Undefined first
    Scenario: Results in undefined
        Given a given step
        Then a then step is missing
            """
        And a file named "success_second.feature" with:
            """
Feature: Success second
    Scenario: Results in success
        Given a given step
        Then a then step
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                |
            | summary                 |
            | --                      |
            | undefined_first.feature |
            | success_second.feature  |
        Then it fails

    Scenario: A passing scenario after a failing scenario does not hide the failure
        Given a file named "fail_feature.feature" with:
            """
Feature: Failing then passing
    Background:
        Given a background step

    Scenario: A failing scenario
        Given a given step
        When a when step
        Then an assertion is raised
        Then a then step

    Scenario: An OK scenario
        Given a given step
        When a when step
        Then a then step
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format |
            | summary  |
            | --       |
            | .        |
        Then it fails
        And the output contains "2 scenarios"
        And the output contains "1 passed"
