Feature: Handle custom step fixtures
    Background: Create feature file that calls step with custom fixture
        Given a file named "custom-step-fixtures.feature" with:
            """
Feature: Handle custom step fixtures
    Scenario: A scenario with a step with custom step fixture
        When a custom fixture is used
            """

    Scenario: Use a custom step fixture that does not assert
        Given a library named "steps" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include "gmock/gmock.h"

            struct CustomFixture : cucumber_cpp::StepBase
            {
            using StepBase::StepBase;
            };

            WHEN_F(CustomFixture, "a custom fixture is used")
            {
            }
            """

        When I run cucumber-cpp-runner with "steps"
        Then it passes

    Scenario: Use custom step fixtures that does not assert
        Given a library named "steps" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include "gmock/gmock.h"

            struct CustomFixture : cucumber_cpp::StepBase
            {
            using StepBase::StepBase;

            ~CustomFixture()
            {
            EXPECT_THAT(true, testing::IsFalse());
            }

            };

            WHEN_F(CustomFixture, "a custom fixture is used")
            {
            }
            """

        When I run cucumber-cpp-runner with "steps"
        Then it fails
