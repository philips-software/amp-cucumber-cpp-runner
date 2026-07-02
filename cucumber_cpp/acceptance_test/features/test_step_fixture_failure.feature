@fail_step_fixture
Feature: Simple feature file
    Rule: Test rule
        Scenario: Test scenario without failing step fixture
            Given step fixture does not fail
        Scenario: Test with failing step fixture
            Given step fixture fails
