@nested_steps
Feature: Nested Steps
    Scenario: Call other steps from within a step
        Given a step calls another step with "cucumber"
        Then the stored string is "cucumber"
