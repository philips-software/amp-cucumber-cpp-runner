#include "cucumber_cpp/CucumberCpp.hpp"
#include <unistd.h>

int main(int argc, char** argv)
{
    cucumber_cpp::Application application{};

    auto** env = environ;

    for (auto i{ 0 }; env[i] != nullptr; ++i)
        fmt::println("env[{}] = {}", i, env[i]);

    return application.Run(argc, argv);
}
