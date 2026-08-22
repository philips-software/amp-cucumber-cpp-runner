@plugin
Feature: Plugin loading with static steps

    @plugin_a
    Scenario: Plugin A provides custom parameter type
        Given a statically linked step
        Then plugin A can read the static context
        When a red is selected
        Then the color name is "red"
        And the static step can read plugin A context

    @plugin_a
    Scenario: Plugin A with different color
        Given a statically linked step
        Then plugin A can read the static context
        When a blue is selected
        Then the color name is "blue"

    @plugin_b_hook
    Scenario: Plugin B provides hooks and steps
        Given a statically linked step
        Then plugin B can read the static context
        And the static step can read plugin B context
        And plugin B hook was executed

    @plugin_a @plugin_b_hook
    Scenario: Both plugins loaded together
        Given a statically linked step
        Then plugin A can read the static context
        And plugin B can read the static context
        When a green is selected
        Then the color name is "green"
        And the static step can read plugin A context
        And the static step can read plugin B context
        And plugin B hook was executed
