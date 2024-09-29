@debug
Feature: Simple feature file

  Background: feature background
    Given a feature background 1
    Given a feature background 2

  Scenario: feature scenario 1
    Given a step

  Scenario: feature scenario 2
    Given a step

  @rule1
  Rule: feature rule 1

    Background: feature rule 1 background
      Given a feature rule 1 background 1
      Given a feature rule 1 background 2

    @rule1scenario1
    Scenario: feature rule 1 scenario 1
      Given a step

    @rule1scenario2
    Scenario: feature rule 1 scenario 2
      Given a step

    @rule2
  Rule: feature rule 2

    Background: feature rule 2 background
      Given a feature rule 2 background 1
      Given a feature rule 2 background 2

    @rule2scenario1
    Scenario: feature rule 2 scenario 1
      Given a step

    @rule2scenario2
    Scenario: feature rule 2 scenario 2
      Given a step
