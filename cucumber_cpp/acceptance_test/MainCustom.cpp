#include "cucumber_cpp/CucumberCpp.hpp"

int main(int argc, char** argv)
{
    cucumber_cpp::Application application{};

    application.CliParser().add_flag("--required", *application.ProgramContext().EmplaceAt<bool>("--required"))->required();
    application.CliParser().add_flag("--failprogramhook", *application.ProgramContext().EmplaceAt<bool>("--failprogramhook"));

    return application.Run(argc, argv);
}
