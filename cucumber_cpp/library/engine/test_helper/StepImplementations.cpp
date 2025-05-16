#include "cucumber_cpp/library/Steps.hpp"
#include "gmock/gmock.h"
#include <cstdint>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>
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

STEP("^This is a step with a ([0-9]+)s delay$", (std::uint32_t delay))
{
    context.InsertAt("std::int64_t", delay);
}

THEN("an ambiguous step")
{
    /* do nothing */
}

STEP("an ambiguous step")
{
    /* do nothing */
}

GIVEN("{int} and {int} are equal", (std::int64_t a, std::int64_t b))
{
    EXPECT_THAT(a, testing::Eq(b));
}

STEP("Step with cucumber expression syntax {float} {string} {int}", (float fl, std::string str, std::int64_t nr))
{
    context.InsertAt("float", fl);
    context.InsertAt("std::string", str);
    context.InsertAt("std::int64_t", nr);
}

GIVEN("I am a nested step")
{
    context.InsertAt("nested", std::string{ "I am a nested step" });
}

WHEN("I call a nested step")
{
    ASSERT_THAT(context.Contains("nested"), testing::IsFalse());

    Given("I am a nested step");
}

THEN("the nested step was called")
{
    EXPECT_THAT(context.Contains("nested"), testing::IsTrue());
}

WHEN("I throw an exception")
{
    throw std::runtime_error{ "Exception thrown" };
}

THEN("the exception is caught")
{
    throw std::runtime_error{ "Should Not Be Thrown" };
}

THEN("the next scenario is executed")
{
    /* do nothing */
}

THEN(R"(An expression with \(parenthesis) should remain as is)")
{
    /* do nothing */
}

THEN(R"(An expression that looks like a function func\({int}, {int}) should keep its parameters)", (std::int64_t a, std::int64_t b))
{
    EXPECT_THAT(a, testing::Eq(1));
    EXPECT_THAT(b, testing::Eq(2));
}

THEN(R"(An expression with \\\(escaped parenthesis\\) should keep the slash)")
{
    /* do nothing */
}

THEN(R"(An expression with \{braces} should remain as is)")
{
    /* do nothing */
}

THEN(R"(An expression with \\\{escaped braces\\} should keep the slash)")
{
    /* do nothing */
}
