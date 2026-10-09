Feature: Selecting scenarios with tag expressions

    Background:
        Given a file named "simple.feature" with:
            """
@smoke
Feature: Simple feature file
    Background:
        Given a background step

    @result:OK
    Scenario: An OK scenario
        Given a given step
        When a when step
        Then a then step

    @result:FAILED
    Scenario: A failing scenario
        Given a given step
        When a when step
        Then an assertion is raised
        Then a then step
            """
        And a file named "subfolder/nested.feature" with:
            """
Feature: Nested feature
    @result:OK
    Scenario: A failing scenario in a subfolder
        Given a given step
        Then an assertion is raised
            """
        And a library named "steps" with:
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

    Scenario: Run only scenarios with a matching tag
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format       |
            | summary        |
            | --tags         |
            | @result:OK     |
            | --no-recursive |
            | --             |
            | .              |
        Then it passes

    Scenario: Combine tags in a tag expression
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format              |
            | summary               |
            | --tags                |
            | @smoke and @result:OK |
            | --                    |
            | .                     |
        Then it passes

    Scenario: Fail when a selected scenario fails
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                  |
            | summary                   |
            | --tags                    |
            | @smoke and @result:FAILED |
            | --                        |
            | .                         |
        Then it fails

    Scenario: Pass when no scenario matches the tag expression
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format    |
            | summary     |
            | --tags      |
            | @invalidtag |
            | --          |
            | .           |
        Then it passes
