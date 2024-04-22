Feature: A feature file with rules
  This is a Simple feature file

  Background:
    Given a background step

  Rule: first rule
    Scenario: first scenario for the first rule
      Given a given step
      When a when step
      Then a then step
      And a step step

    Scenario: second scenario for the first rule
      Given a given step
      When a when step
      Then a then step
      And a step step

  Rule: second rule
    Background:
      Given another background step

    Scenario: first scenario for the second rule
      Given a given step
      When a when step
      Then a then step
      And a step step
