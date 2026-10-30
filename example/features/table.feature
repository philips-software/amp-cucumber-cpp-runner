@table_argument
Feature: Steps can access step table arguments

    Scenario Outline: Step with table argument can access table data
            """
            you can have a docstring
            """
        When a step stores the value at row <row> and column <column> from the table:
            | name    | age | city        |
            | Alice   | 30  | New York    |
            | Bob     | 25  | Los Angeles |
            | Charlie | 35  | Chicago     |
        Then the value should be "<expected_value>"

        Examples:
            | row | column | expected_value |
            | 1   | 0      | Alice          |
            | 1   | 1      | 30             |
            | 1   | 2      | New York       |
            | 2   | 0      | Bob            |
            | 2   | 1      | 25             |
            | 2   | 2      | Los Angeles    |
            | 3   | 0      | Charlie        |
            | 3   | 1      | 35             |
            | 3   | 2      | Chicago        |
