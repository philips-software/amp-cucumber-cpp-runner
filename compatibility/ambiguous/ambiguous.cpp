#include "cucumber_cpp/CucumberCpp.hpp"
#include <string>

STEP(R"(^a (.*?) with (.*?)$)", (const std::string& arg1, const std::string& arg2))
{
    // no-op
}

STEP(R"(^a step with (.*)$)", (const std::string& arg1))
{
    // no-op
}
