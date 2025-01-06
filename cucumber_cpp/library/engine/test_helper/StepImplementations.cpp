#include "cucumber_cpp/library/Steps.hpp"
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

GIVEN("This is a GIVEN step")
{
    /* do nothing */
}

WHEN("This is a WHEN step")
{
    /* do nothing */
}

THEN("This is a THEN step")
{
    /* do nothing */
}

STEP("This is a STEP step")
{
    /* do nothing */
}

STEP("This is a step with a ([0-9]+)s delay", (std::uint32_t delay))
{
    context.InsertAt("std::uint32_t", delay);
}

THEN("an ambiguous step")
{
    /* do nothing */
}

STEP("an ambiguous step")
{
    /* do nothing */
}

GIVEN("{int} and {int} are equal", (std::uint32_t a, std::uint32_t b))
{
    EXPECT_THAT(a, testing::Eq(b));
}

STEP("Step with cucumber expression syntax {float} {string} {int}", (float fl, std::string str, std::uint32_t nr))
{
    context.InsertAt("float", fl);
    context.InsertAt("std::string", str);
    context.InsertAt("std::uint32_t", nr);
}
