#include "cucumber_cpp/CucumberCpp.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

GIVEN("a background step")
{
    // empty
}

GIVEN("a given step")
{
    // empty
}

WHEN("a when step")
{
    // empty
}

THEN("a then step")
{
    // empty
}

STEP("a step step")
{
    // empty
}

WHEN("I print {string}", (const std::string& str))
{
    std::cout << std::format("print: {}\n", str);
}

THEN("an assertion is raised")
{
    ASSERT_THAT(false, testing::IsTrue());
}

THEN("two expectations are raised")
{
    EXPECT_THAT(0, testing::Eq(1));
    EXPECT_THAT(1, testing::Eq(0));
}

GIVEN("a ← tuple\\({float}, {float}, {float}, {float}\\)", (float /* unused */, float /* unused */, float /* unused */, float /* unused */))
{
    // empty
}

THEN("this should be skipped")
{
    FAIL();
}

GIVEN("Next block of text enclosed in \"\"\" characters")
{

    ASSERT_THAT(docString, testing::Eq("Multiline\nDocstring"));
}

WHEN("this step is being used")
{
    // empty
}

WHEN("this step is not being used")
{
    // empty
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

GIVEN("{int} and {int} are equal", (std::int32_t a, std::int32_t b))
{
    EXPECT_THAT(a, testing::Eq(b));
}
