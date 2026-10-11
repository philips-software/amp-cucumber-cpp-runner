Feature: Selecting feature files by path

    Scenario: Run all feature files in a folder
        Given a file named "outside.feature" with:
            """
Feature: Outside the selected folder
    Scenario: Not selected
        Given a given step
            """
        And a file named "subfolder/test1.feature" with:
            """
Feature: test1 feature
    Scenario: test1 scenario
        Given a given step
            """
        And a file named "subfolder/test2.feature" with:
            """
Feature: test2 feature
    Scenario: test2 scenario
        Given a given step
            """
        And a library named "steps" with:
            """
            #include "cucumber_cpp/Steps.hpp"

            GIVEN("a given step")
            {
            }
            """
        When I run cucumber-cpp-runner with "steps" and arguments:
            | --format  |
            | summary   |
            | --        |
            | subfolder |
        Then it passes
        And the output contains "2 scenarios"
        And the output contains "2 passed"
