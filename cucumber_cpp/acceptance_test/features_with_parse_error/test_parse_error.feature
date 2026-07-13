Feature: a parse error is printed to cerr
    Scenario: a parse error is printed to cerr
        Given a feature file with a parse error
        when this line is a parse error (when should be When)
        Then this should be skipped
