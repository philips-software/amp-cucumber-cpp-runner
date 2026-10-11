Feature: Step definitions

    Background:
        Given a library named "steps" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include "gmock/gmock.h"
            #include <string>

            GIVEN("a ← tuple\\({float}, {float}, {float}, {float}\\)", (float, float, float, float))
            {
            }

            WHEN("this step is being used")
            {
            }

            WHEN("this step is not being used")
            {
            }

            GIVEN("a step calls another step with {string}", (const std::string& str))
            {
            Step("I store \"" + str + "\"");
            }

            GIVEN("I store {string}", (const std::string& str))
            {
            Step("I store \"" + str + "\" again");
            }

            GIVEN("I store {string} again", (const std::string& str))
            {
            context.InsertAt("storedstring", str);
            }

            THEN("the stored string is {string}", (const std::string& expected))
            {
            EXPECT_THAT(context.Get<std::string>("storedstring"), testing::StrEq(expected));
            }

            GIVEN("step fixture does not fail")
            {
            }

            struct FailingStepFixture : cucumber_cpp::StepBase
            {
            using StepBase::StepBase;

            bool nonExistentKey = context.Get<bool>("nonExistentKey");
            };

            GIVEN_F(FailingStepFixture, "step fixture fails")
            {
            }

            GIVEN("a nested step that fails")
            {
            ASSERT_THAT(false, testing::IsTrue());
            }

            GIVEN("a step calls another step that will fail")
            {
            Step("a nested step that fails");
            }

            THEN("this should be skipped")
            {
            FAIL();
            }
            """

    Scenario: Steps can match unicode characters
        Given a file named "unicode.feature" with:
            """
Feature: Test for unicode characters
    Scenario: Can match unicode characters
        Given a ← tuple(4.3, -4.2, 3.1, 1.0)
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format |
            | summary  |
            | --       |
            | .        |
        Then it passes
        And the output contains "1 scenario"
        And the output contains "1 passed"

    Scenario: Unused steps are not reported by default
        Given a file named "unused_steps.feature" with:
            """
Feature: Test for unused step detection
    Scenario: One step used, one step not used
        When this step is being used
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format |
            | summary  |
            | pretty   |
            | --       |
            | .        |
        Then it passes
        And the output does not contain "The following steps have not been used:"

    Scenario: Steps can call other steps
        Given a file named "nested_steps.feature" with:
            """
Feature: Nested Steps
    Scenario: Call other steps from within a step
        Given a step calls another step with "cucumber"
        Then the stored string is "cucumber"
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format |
            | summary  |
            | pretty   |
            | --       |
            | .        |
        Then it passes

    Scenario: A failing step fixture fails the step
        Given a file named "step_fixture.feature" with:
            """
Feature: Step fixtures
    Scenario: Test scenario without failing step fixture
        Given step fixture does not fail

    Scenario: Test with failing step fixture
        Given step fixture fails
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                         |
            | summary                          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it fails
        And the output contains:
            """
            key not found: "nonExistentKey"
            """
        And the output contains "2 scenarios (1 passed, 1 failed)"
        And the output contains "2 steps (1 passed, 1 failed)"

    Scenario: Failures in nested steps propagate to the calling step
        Given a file named "nested_failing_steps.feature" with:
            """
Feature: Nested failing steps
    Scenario: Call a failing step from within a step
        When a step calls another step that will fail
        Then this should be skipped
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format                         |
            | summary                          |
            | --format-options                 |
            | { "summary": {"theme":"plain"} } |
            | --                               |
            | .                                |
        Then it fails
        And the output contains:
            """
            FAILED nested step: "* a nested step that fails"
            """
        And the output contains "Value of: false"
        And the output contains "Expected: is true"
        And the output contains "Actual: false (of type bool)"
        And the output contains "2 steps (1 skipped, 1 failed)"
