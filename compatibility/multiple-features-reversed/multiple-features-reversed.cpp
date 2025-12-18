#include "cucumber_cpp/CucumberCpp.hpp"
#include <string>

STEP(R"(an order for {string})", ([[maybe_unused]] const std::string& order))
{
    // no-op
}
