Feature: Step keywords

    Background:
        Given a library named "print" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include <iostream>
            #include <string>

            WHEN("I print {string}", (const std::string& str))
            {
            std::cout << "print: " << str << "\n";
            }
            """

    Scenario: The And keyword continues the previous step type
        Given a file named "and.feature" with:
            """
Feature: And keyword
    Scenario: To test the keyword "And"
        When I print "--when--"
        And I print "--and--"
            """
        When I run cucumber-cpp-runner with "print" and arguments:
            | --format |
            | summary  |
            | --       |
            | .        |
        Then it passes
        And the output contains "print: --when--"
        And the output contains "print: --and--"

    Scenario: The But keyword continues the previous step type
        Given a file named "but.feature" with:
            """
Feature: But keyword
    Scenario: To test the keyword "But"
        When I print "--when--"
        But I print "--but--"
            """
        When I run cucumber-cpp-runner with "print" and arguments:
            | --format |
            | summary  |
            | --       |
            | .        |
        Then it passes
        And the output contains "print: --when--"
        And the output contains "print: --but--"

    Scenario: The asterisk keyword continues the previous step type
        Given a file named "asterisk.feature" with:
            """
Feature: Asterisk keyword
    Scenario: To test the keyword "*"
        When I print "--when--"
        * I print "--asterisk--"
            """
        When I run cucumber-cpp-runner with "print" and arguments:
            | --format |
            | summary  |
            | --       |
            | .        |
        Then it passes
        And the output contains "print: --when--"
        And the output contains "print: --asterisk--"
