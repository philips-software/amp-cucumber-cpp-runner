@smoke
Feature: Simple feature file
  This is a Simple feature file

  Background:
    Given a background step

  Rule: Scenarios that have a rule applied

    @result:OK @printme
    Scenario Outline: Can substract
      Given there are <x> cucumbers
        | foo | bar | asdasd | ad     |
        | boz | boo | asd    | asdasd |
      When I eat <y> cucumbers
      Then I should have <z> cucumbers

      Examples:
        | x  | y | z |
        | 10 | 5 | 5 |
        | 11 | 3 | 8 |

    @result:FAILED
    Scenario Outline: Is a dingus
      Given there are <x> cucumbers
      When I eat <y> cucumbers
      Then I should have <z> cucumbers
      And this step should be skipped
        | foo | bar |
        | boz | boo |

      Examples:
        | x  | y | z |
        | 10 | 4 | 5 |
        | 11 | 3 | 6 |

    @result:UNDEFINED
    Scenario: a scenario with a missing step
      Given there are <x> cucumbers
      When I eat <y> cucumbers
      Then I should have <z> cucumbers left
      And this step should be skipped
      Examples:
        | x  | y | z |
        | 10 | 5 | 5 |
        | 11 | 3 | 8 |

  Rule: Scenarios that have another rule applied

    @result:OK @printme
    Scenario Outline: Can substract
      Given there are <x> cucumbers
        | foo | bar | asdasd | ad     |
        | boz | boo | asd    | asdasd |
      When I eat <y> cucumbers
      Then I should have <z> cucumbers

      Examples:
        | x  | y | z |
        | 10 | 5 | 5 |
        | 11 | 3 | 8 |

    @result:FAILED
    Scenario Outline: Is a dingus
      Given there are <x> cucumbers
      When I eat <y> cucumbers
      Then I should have <z> cucumbers
      And this step should be skipped
        | foo | bar |
        | boz | boo |

      Examples:
        | x  | y | z |
        | 10 | 4 | 5 |
        | 11 | 3 | 6 |

    @result:UNDEFINED
    Scenario: a scenario with a missing step
      Given there are <x> cucumbers
      When I eat <y> cucumbers
      Then I should have <z> cucumbers left
      And this step should be skipped
      Examples:
        | x  | y | z |
        | 10 | 5 | 5 |
        | 11 | 3 | 8 |
