Feature: Formatters

    Scenario: Only known formatters are accepted
        When I run cucumber-cpp-runner with arguments:
            | --format     |
            | doesnotexist |
        Then it fails
        And the output contains "--format: 'doesnotexist' is not a valid formatter"

    Scenario: The usage formatter reports used and unused steps
        Given a file named "usage.feature" with:
            """
Feature: Usage
    Scenario: A scenario using one step
        Given this step is used
            """
        And a library named "usage" with:
            """
            #include "cucumber_cpp/Steps.hpp"

            STEP("This step is unused")
            {
            }

            STEP("this step is used")
            {
            }
            """
        When I run cucumber-cpp-runner with "usage" and arguments:
            | --format |
            | usage    |
            | --       |
            | .        |
        Then it passes
        And the output contains "│ this step is used   │ "
        And the output contains "│   this step is used │ "
        And the output contains "│ This step is unused │ UNUSED   │"
        When I run cucumber-cpp-runner with "usage" and arguments:
            | --format  |
            | usage     |
            | --dry-run |
            | --        |
            | .         |
        Then it passes
        And the output contains "│ this step is used   │ -        │"
        And the output contains "│   this step is used │ -        │"
        And the output contains "│ This step is unused │ UNUSED   │"
        When I run cucumber-cpp-runner with "usage" and arguments:
            | --format                         |
            | usage                            |
            | --dry-run                        |
            | --format-options                 |
            | { "usage" : {"theme": "plain"} } |
            | --                               |
            | .                                |
        Then it passes
        And the output contains "| this step is used   | -        |"
        And the output contains "|   this step is used | -        |"
        And the output contains "| This step is unused | UNUSED   |"
