Feature: Dry run

    Background:
        Given a file named "results.feature" with:
            """
Feature: Scenario results
    @result:FAILED
    Scenario: A failing scenario
        Given a given step
        Then an assertion is raised

    @result:FAILED
    Scenario: A throwing scenario
        Given a given step
        Then an exception is thrown

    @result:UNDEFINED
    Scenario: A scenario with undefined step
        Given a missing step
        Then a then step
            """
        And a library named "steps" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include "gmock/gmock.h"
            #include <stdexcept>

            GIVEN("a given step")
            {
            }

            THEN("a then step")
            {
            }

            THEN("an assertion is raised")
            {
            ASSERT_THAT(false, testing::IsTrue());
            }

            THEN("an exception is thrown")
            {
            throw std::runtime_error{ "This is a custom exception" };
            }
            """

    Scenario: A dry run does not execute failing steps
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format       |
            | summary        |
            | --tags         |
            | @result:FAILED |
            | --             |
            | .              |
        Then it fails
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format       |
            | summary        |
            | --tags         |
            | @result:FAILED |
            | --dry-run      |
            | --             |
            | .              |
        Then it passes

    Scenario: A dry run still reports undefined steps
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format          |
            | summary           |
            | --tags            |
            | @result:UNDEFINED |
            | --                |
            | .                 |
        Then it fails
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                         |
            | summary                          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --tags                           |
            | @result:UNDEFINED                |
            | --dry-run                        |
            | --                               |
            | .                                |
        Then it passes
        And the output contains "Given a missing step"
