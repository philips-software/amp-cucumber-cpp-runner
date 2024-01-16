#include "cucumber-cpp/Application.hpp"
#include <span>

int main(int argc, char** argv)
{
    cucumber_cpp::Application application{ std::span(const_cast<const char**>(argv), argc) };

    auto args = application.GetForwardArgs();

    application.RunFeatures();
    application.GenerateReports();

    return application.GetExitCode();
}
