@expose_scenario_info
Feature: Scenario begin and end hooks can expose scenario information
    @store_scenario_info
    Rule: Scenario begin and end hooks store the scenario information
        Background:
            Then the "ScenarioInfoHook" has the name "Scenario with @tag-a" or "Scenario with @tag-b"
            And the "ScenarioInfoHook" has the tag "@tag-a" or "@tag-b"
            And the "StepHookInfo" has the name "Scenario with @tag-a" or "Scenario with @tag-b"
            And the "StepHookInfo" has the tag "@tag-a" or "@tag-b"

        Scenario Outline: Scenario with <tag>
            Then the "<scope>" has the name "Scenario with <tag>"
            And the "<scope>" has the tags "@expose_scenario_info" and "<tag>"

            @tag-a
            Examples:
                | scope            | tag    |
                | ScenarioInfoHook | @tag-a |
                | StepHookInfo     | @tag-a |

            @tag-b
            Examples:
                | scope            | tag    |
                | ScenarioInfoHook | @tag-b |
                | StepHookInfo     | @tag-b |

    @no_store_scenario_info
    Rule: scenario begin and end hooks do not store the scenario information
        Scenario: Scenario information is available
            Then the "ScenarioInfoHook" is not available
            And the "StepHookInfo" is not available
