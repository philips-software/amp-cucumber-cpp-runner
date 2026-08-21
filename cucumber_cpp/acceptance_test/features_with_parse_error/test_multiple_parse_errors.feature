Feature: multiple parse errors in a single feature are all reported

    Scenario: two invalid lines produce two parse errors
        Given a valid step
        when this is the first parse error
        Then another valid step
when this is the second parse error
