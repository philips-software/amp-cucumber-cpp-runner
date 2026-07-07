Feature: Nested Steps
    @nested_steps
    Scenario: Call other steps from within a step
        Given a step calls another step with "cucumber"
        Then the stored string is "cucumber"

    @nested_failing_steps
    Scenario: Call other steps from within a step
        When a step calls another step that will fail
        Then this should be skipped
