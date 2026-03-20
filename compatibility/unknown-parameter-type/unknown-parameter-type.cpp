#include "cucumber_cpp/Steps.hpp"
#include <gtest/gtest.h>

struct Airport
{};

GIVEN(R"({airport} is closed because of a strike)", ([[maybe_unused]] const Airport& airport))
{
    FAIL() << "Should not be called because airport parameter type has not been defined";
}
