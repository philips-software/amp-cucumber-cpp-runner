@bats
Feature: Test scenario and step hook bindings

  Background:
    Given a background step

  @scenariohook @stephook
  Scenario: Run Scenario and Step hooks
    Given a step

  @scenariohook
  Scenario: Run only Scenario hooks
    Given a step

  @stephook
  Scenario: Run only Step hooks
    Given a step
