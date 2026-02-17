#include "cucumber_cpp/CucumberCpp.hpp"
#include <string>

STEP(R"(^a (.*?)(?: and a (.*?))?(?: and a (.*?))?$)", (const std::string& vegetable1, const std::string& vegetable2, const std::string& vegetable3))
{
    // no-op
}
