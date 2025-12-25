#include "cucumber_cpp/CucumberCpp.hpp"
#include <gtest/gtest.h>

struct Airport
{};

GIVEN(R"({airport} is closed because of a strike)", (const Airport& airport))
{
    FAIL() << "Should not be called because airport parameter type has not been defined";
}
