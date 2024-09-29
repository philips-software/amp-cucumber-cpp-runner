#include "cucumber_cpp/Application.hpp"
#include <string>

int main(int argc, char** argv)
{
    cucumber_cpp::Application application{};

    application.CliParser().add_option("--com", *application.ProgramContext().EmplaceAt<std::string>("--com"))->required();
    auto* stOpt = application.CliParser().add_flag("--st", *application.ProgramContext().EmplaceAt<bool>("--st"));
    auto* nordicOpt = application.CliParser().add_flag("--nordic", *application.ProgramContext().EmplaceAt<bool>("--nordic"));

    stOpt->excludes(nordicOpt);
    nordicOpt->excludes(stOpt);

    return application.Run(argc, argv);
}
