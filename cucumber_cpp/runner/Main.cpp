#include "cucumber_cpp/library/Application.hpp"
#include <span>

int main(int argc, char** argv)
{
    cucumber_cpp::Application application{};

    return application.Run(argc, argv);
}
