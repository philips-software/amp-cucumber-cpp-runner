#include "cucumber_cpp/library/Steps.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>
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
    std::cout << "print: " << str;
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

WHEN("I print {string} with value {int}", (const std::string& str, std::int32_t value))
{
    std::cout << "print: " << str << " with value " << value;
}
