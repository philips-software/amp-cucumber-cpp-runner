#include "cucumber-cpp/Application.hpp"
#include <span>

int main(int argc, char** argv)
{
    cucumber_cpp::Application application{};

    application.CliParser().add_option("--com", *application.ProgramContext().EmplaceAt<std::string>("--com"))->required();
    auto* stOpt = application.CliParser().add_flag("--st", *application.ProgramContext().EmplaceAt<bool>("--st"));
    auto* nordicOpt = application.CliParser().add_flag("--nordic", *application.ProgramContext().EmplaceAt<bool>("--nordic"));

    stOpt->excludes(nordicOpt);
    nordicOpt->excludes(stOpt);

    auto retval = application.Run(argc, argv);

    return retval;
}
