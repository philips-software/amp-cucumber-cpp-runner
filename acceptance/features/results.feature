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

            GIVEN("a step that fails only on attempt {int}", (std::int32_t failingAttempt))
            {
            static int attempts = 0;
            ASSERT_THAT(++attempts, testing::Ne(failingAttempt));
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

    Scenario Outline: Repetition executes the fixed count and retains failures
        Given a file named "repeat.feature" with:
            """
Feature: Repeated executions
    Scenario: Repeated scenario
        Given <step>
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --repeat                         |
            | 3                                |
            | --format                         |
            | summary                          |
            | message:messages.ndjson          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it <result>
        And the output contains "1 scenario"
        And the message file "messages.ndjson" contains 3 attempts for scenario "Repeated scenario"

        Examples:
            | step                                | result |
            | a given step                        | passes |
            | an assertion is raised              | fails  |
            | a step that fails only on attempt 1 | fails  |
            | a step that fails only on attempt 2 | fails  |
            | a step that fails only on attempt 3 | fails  |

    Scenario: Repetition is restricted by its tag filter
        Given a file named "repeat.feature" with:
            """
Feature: Tagged repetition
    @sample
    Scenario: Repeated scenario
        Given a given step
    @other
    Scenario: Ordinary scenario
        Given a given step
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --repeat                         |
            | 3                                |
            | --repeat-tag-filter              |
            | @sample                          |
            | --format                         |
            | summary                          |
            | message:messages.ndjson          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it passes
        And the output contains "2 scenarios"
        And the message file "messages.ndjson" contains 3 attempts for scenario "Repeated scenario"
        And the message file "messages.ndjson" contains 1 attempts for scenario "Ordinary scenario"

    Scenario: Fail fast finishes repetition before skipping the next scenario
        Given a file named "repeat.feature" with:
            """
Feature: Repetition with fail fast
    Scenario: Failing scenario
        Then an assertion is raised
    Scenario: Skipped scenario
        Given a given step
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --repeat                |
            | 3                       |
            | --fail-fast             |
            | --format                |
            | message:messages.ndjson |
            | --                      |
            | .                       |
        Then it fails
        And the message file "messages.ndjson" contains 3 attempts for scenario "Failing scenario"
        And the message file "messages.ndjson" contains 1 attempts for scenario "Skipped scenario"

    Scenario: Dry run does not repeat scenarios
        Given a file named "repeat.feature" with:
            """
Feature: Repetition with dry run
    Scenario: Dry run scenario
        Then an assertion is raised
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --repeat                |
            | 3                       |
            | --dry-run               |
            | --format                |
            | message:messages.ndjson |
            | --                      |
            | .                       |
        Then it passes
        And the message file "messages.ndjson" contains 1 attempts for scenario "Dry run scenario"

    Scenario Outline: Attempt mode survives a configuration dump and reload
        Given a file named "repeat.feature" with:
            """
Feature: Configured executions
    Scenario: Configured scenario
        Given a given step
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | <option>                |
            | <count>                 |
            | --dump-config           |
            | --format                |
            | message:messages.ndjson |
            | --                      |
            | .                       |
        Then it passes
        When I run cucumber-cpp-runner with arguments:
            | --config      |
            | cucumber.toml |
        Then it passes
        And the message file "messages.ndjson" contains <attempts> attempts for scenario "Configured scenario"

        Examples:
            | option   | count | attempts |
            | --repeat | 3     | 3        |
            | --retry  | 2     | 1        |
