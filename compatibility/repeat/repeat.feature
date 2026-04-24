Feature: Repeat
  Some Cucumber implementations support a Repeat mechanism, where scenarios
  can be repeated a given number of times for performance measurement.

  Scenario: Untagged scenarios run once even when repeat is set
    Given a step that counts executions

  @Performance
  Scenario: Tagged scenarios are repeated when matching the repeat tag filter
    Given a step that counts tagged executions
