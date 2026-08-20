Feature: Repeat
  Some Cucumber implementations support running a scenario a fixed number of
  times unconditionally, for sampling or measurement. Unlike Retry, every
  execution runs regardless of the outcome, and the executions surface as
  incrementing attempts on the same test case.

  Scenario: A repeated scenario runs the requested number of times
    Given a step that always passes
