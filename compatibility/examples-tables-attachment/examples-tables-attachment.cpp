#include "cucumber_cpp/Steps.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <source_location>
#include <string>

namespace
{
    const std::filesystem::path compatibilityPath = std::filesystem::path{ KIT_FOLDER };
}

WHEN(R"(a JPEG image is attached)")
{
    std::ifstream jpegFile{ compatibilityPath / "cucumber.jpeg", std::ios::binary };
    Attach(jpegFile, "image/jpeg");
}

WHEN(R"(a PNG image is attached)")
{
    std::ifstream pngFile{ compatibilityPath / "cucumber.png", std::ios::binary };
    Attach(pngFile, "image/png");
}
