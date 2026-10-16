Feature: Custom command line arguments

    Scenario: A custom executable rejects a missing mandatory argument
        Given an executable named "custom_cli" with:
            """
            #include "cucumber_cpp/CucumberCpp.hpp"

            int main(int argc, char** argv)
            {
            cucumber_cpp::Application application{};
            application.CliParser().add_flag("--required", *application.ProgramContext().EmplaceAt<bool>("--required"))->required();
            return application.Run(argc, argv);
            }
            """
        When I run the executable "custom_cli" with arguments:
            | --format |
            | summary  |
            | --       |
            | .        |
        Then it fails
        And the output contains "--required is required"
