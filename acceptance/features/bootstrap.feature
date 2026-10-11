Feature: Bootstrap cucumber-cpp-runner implementation

    Scenario: Build a plugin library and run a feature file
        Given a file named "a feature file.feature" with:
            """
Feature: Child feature
    Scenario: Passing scenario
        Given a runner step
        Then it succeeds
            """

        Given a library named "my library" with:
            """
            #include "cucumber_cpp/Steps.hpp"

            GIVEN("a runner step")
            {
            // empty
            }

            THEN("it succeeds")
            {
            // empty
            }
            """

        When I run cucumber-cpp-runner with "my library"
        Then it passes

    Scenario: Build a plugin library and run a failing feature file
        Given a file named "a failing feature.feature" with:
            """
Feature: Child failure
    Scenario: Failing scenario
        Then it fails in child
            """

        Given a library named "mylibrary_failing" with:
            """
            #include "cucumber_cpp/Steps.hpp"
            #include <stdexcept>

            THEN("it fails in child")
            {
            throw std::runtime_error{"intentional child failure"};
            }
            """

        When I run cucumber-cpp-runner with "mylibrary_failing"
        Then it fails
