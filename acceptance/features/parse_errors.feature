Feature: Parse errors

    Scenario: A parse error is reported with its location
        Given a file named "test_parse_error.feature" with:
            """
Feature: a parse error is printed to cerr
    Scenario: a parse error is printed to cerr
        Given a feature file with a parse error
        when this line is a parse error (when should be When)
        Then this should be skipped
            """
        When I run cucumber-cpp-runner with arguments:
            | test_parse_error.feature |
        Then it fails
        And the output contains:
            """
            Parse error in: "test_parse_error.feature:4:
            """
        And the output contains "got 'when this line is a parse error (when should be When)'"

    Scenario: Multiple parse errors in a single feature are all reported
        Given a file named "test_multiple_parse_errors.feature" with:
            """
Feature: multiple parse errors in a single feature are all reported

    Scenario: two invalid lines produce two parse errors
        Given a valid step
        when this is the first parse error
        Then another valid step
            when this is the second parse error
            """
        When I run cucumber-cpp-runner with arguments:
            | test_multiple_parse_errors.feature |
        Then it fails
        And the output contains "test_multiple_parse_errors.feature:5:"
        And the output contains "got 'when this is the first parse error'"
        And the output contains "test_multiple_parse_errors.feature:7:"
        And the output contains "got 'when this is the second parse error'"
