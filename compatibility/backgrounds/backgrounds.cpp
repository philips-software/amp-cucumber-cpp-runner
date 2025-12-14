#include "cucumber_cpp/CucumberCpp.hpp"
#include <string>

STEP(R"(an order for {string})", ([[maybe_unused]] const std::string& item))
{
    // no-op
}

STEP(R"(an action)")
{
    // no-op
}

STEP(R"(an outcome)")
{
    // no-op
}
