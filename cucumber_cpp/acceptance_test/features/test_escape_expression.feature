@smoke
Feature: Cucumber expressions escape sequence
    Scenario: To test parenthesized expression
        Given a given step
        Then An expression with (parenthesis) should remain as is
        Then An expression that looks like a function func(1, 2) should keep its parameters
        Then An expression with \(escaped parenthesis\) should keep the slash

    Scenario: To test braced expression
        Given a given step
        Then An expression with {braces} should remain as is
        Then An expression with \{escaped braces\} should keep the slash
