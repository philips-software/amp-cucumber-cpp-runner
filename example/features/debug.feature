@debug
Feature: Debug simple feature file

  Background: background
    Given a feature background 1
    Given a feature background 2

  Scenario: scenario 1
    Given a step

  Scenario: scenario 2
    Given a step

  @rule1
  Rule: rule 1

    Background: background
      Given a feature rule 1 background 1
      Given a feature rule 1 background 2

    @rule1scenario1
    Scenario: scenario 1
      Given a step

    @rule1scenario2
    Scenario: scenario 2
      Given a step

    @rule2
  Rule: rule 2

    Background: background
      Given a feature rule 2 background 1
      Given a feature rule 2 background 2

    @rule2scenario1
    Scenario: scenario 1
      Given a step

    @rule2scenario2
    Scenario: scenario 2
      Given a step
