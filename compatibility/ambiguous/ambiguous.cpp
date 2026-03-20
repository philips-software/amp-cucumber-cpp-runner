#include "cucumber_cpp/Steps.hpp"
#include <string>

STEP(R"(^a (.*?) with (.*?)$)", ([[maybe_unused]] const std::string& arg1, [[maybe_unused]] const std::string& arg2))
{
    // no-op
}

STEP(R"(^a step with (.*?)$)", ([[maybe_unused]] const std::string& arg1))
{
    // no-op
}
