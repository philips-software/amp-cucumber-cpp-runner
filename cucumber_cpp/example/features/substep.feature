@substep
Feature: Feature with a step that will call another step

    Scenario: Scenario under a rule
        When a step calls another step
        Then the called step is executed
