#include "cucumber_cpp/CucumberCpp.hpp"

int main(int argc, char** argv)
{
    cucumber_cpp::Application application{};

    application.CliParser().add_flag("--required", *application.ProgramContext().EmplaceAt<bool>("--required"))->required();

    return application.Run(argc, argv);
}
