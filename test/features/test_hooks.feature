@bats
Feature: Test scenario and step hook bindings

  Background:
    Given a background step

  @scenariohook @stephook
  Scenario: Run Scenario and Step hooks
    Given a given step
    When a when step
    Then a then step

  @scenariohook
  Scenario: Run only Scenario hooks
    Given a given step
    When a when step
    Then a then step

  @stephook
  Scenario: Run only Step hooks
    Given a given step
    When a when step
    Then a then step
