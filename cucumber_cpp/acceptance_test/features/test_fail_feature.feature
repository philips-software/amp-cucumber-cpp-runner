@fail_feature
Feature: Simple feature file
    Background:
        Given a background step

    Scenario: A failing scenario
        Given a given step
        When a when step
        Then an assertion is raised
        Then a then step

    Scenario: An OK scenario
        Given a given step
        When a when step
        Then a then step
