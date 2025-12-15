#include "cucumber_cpp/CucumberCpp.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <source_location>
#include <string>

namespace
{
    const std::filesystem::path currentCompileDir = std::filesystem::path{ std::source_location::current().file_name() }.parent_path();
}

WHEN(R"(a JPEG image is attached)")
{
    std::ifstream jpegFile{ currentCompileDir / "cucumber.jpeg", std::ios::binary };
    Attach(jpegFile, "image/jpeg");
}

WHEN(R"(a PNG image is attached)")
{
    std::ifstream pngFile{ currentCompileDir / "cucumber.png", std::ios::binary };
    Attach(pngFile, "image/png");
}
