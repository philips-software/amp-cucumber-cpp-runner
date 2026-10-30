Feature: Plugin example
    Demonstrates steps loaded from a dynamic library plugin.

    Scenario: Greeting
        Given the greeter says "Hello"
        When the user responds
        Then the conversation is complete
