@bats
Feature: Test scenario and step hook bindings

  @program_hooks
  Scenario: Run failing Scenario hooks before
    Given a given step

  @fail_scenariohook_before
  Scenario: Run failing Scenario hooks before
    Given a given step

  @fail_scenariohook_after
  Scenario: Run failing Scenario hooks after
    Given a given step

  @throw_scenariohook
  Scenario: Run throwing Scenario hooks
    Given a given step

  Rule: These scenarios have a background

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
