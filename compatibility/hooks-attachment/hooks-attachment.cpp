#include "cucumber_cpp/CucumberCpp.hpp"
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <ios>
#include <source_location>

namespace
{
    const std::filesystem::path currentCompileDir = std::filesystem::path{ std::source_location::current().file_name() }.parent_path();
}

HOOK_BEFORE_SCENARIO()
{
    std::ifstream svgFile{ currentCompileDir / "cucumber.svg", std::ios::binary };
    Attach(svgFile, "image/svg+xml");
}

WHEN(R"(a step passes)")
{
    // no-op
}

HOOK_AFTER_SCENARIO()
{
    std::ifstream svgFile{ currentCompileDir / "cucumber.svg", std::ios::binary };
    Attach(svgFile, "image/svg+xml");
}
