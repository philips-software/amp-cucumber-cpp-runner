@smoke
Feature: Simple feature file
  Background:
    Given a background step

  @result:OK
  Scenario: An OK scenario
    Given a given step
    When a when step
    Then a then step

  @result:FAILED
  Scenario: A failing scenario
    Given a given step
    When a when step
    Then an assertion is raised
    Then a then step

  @result:UNDEFINED
  Scenario: A scenario with undefined step
    Given a given step
    When a when step
    Then a when step
    Then a then step
