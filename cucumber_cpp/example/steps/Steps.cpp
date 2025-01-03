#include "cucumber_cpp/library/Steps.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/engine/TestFailureHandler.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <iostream>
#include <string>

GIVEN(R"(a background step)")
{
    std::cout << "\nthis is a background step";
}

GIVEN(R"(a simple data table)")
{
    std::cout << "row0.col0: " << table[0][0].As<std::string>() << "\n";
    std::cout << "row0.col1: " << table[0][1].As<std::string>() << "\n";

    std::cout << "row1.col0: " << table[1][0].As<std::string>() << "\n";
    std::cout << "row1.col1: " << table[1][1].As<std::string>() << "\n";
}

GIVEN(R"(there are {int} cucumbers)", (std::uint32_t num))
{
    context.InsertAt("cucumbers_before", num);
}

STEP(R"(I eat {int} cucumbers)", (std::uint32_t num))
{
    context.InsertAt("cucumbers_eaten", num);
}

THEN(R"(^I should have ([0-9]+) cucumbers$)", (std::uint32_t num))
{
    const auto& before = context.Get<std::uint32_t>("cucumbers_before");
    const auto& eaten = context.Get<std::uint32_t>("cucumbers_eaten");

    const auto actual = before - eaten;

    ASSERT_THAT(actual, testing::Eq(num));
}

THEN(R"(I should have ([0-9]+) cucumbers left)", (std::uint32_t num))
{
    const auto& before = context.Get<std::uint32_t>("cucumbers_before");
    const auto& eaten = context.Get<std::uint32_t>("cucumbers_eaten");

    const auto actual = before - eaten;

    ASSERT_THAT(actual, testing::Eq(num));
}

STEP(R"(a data table with a single cell)", (int i, double d))
{
    /* no body, example only */
}

STEP(R"(a data table with different fromatting)")
{
    /* no body, example only */
}

STEP(R"(a data table with an empty cell)")
{
    /* no body, example only */
}

STEP(R"(a data table with comments and newlines inside)")
{
    /* no body, example only */
}
