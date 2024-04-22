#include "cucumber-cpp/Steps.hpp"
#include "cucumber-cpp-example/fixtures/Fixture.hpp"
#include "cucumber-cpp/Context.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <any>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

GIVEN(R"(a background step)")
{
    std::cout << "\nthis is a background step\n";
}

GIVEN("another background step")
{
    /* keep empty */
}

GIVEN("a step")
{
    /* keep empty */
}

GIVEN("a given step")
{
    /* keep empty */
}

WHEN("a when step")
{
    /* keep empty */
}

THEN("a then step")
{
    /* keep empty */
}

STEP("a step step")
{
    /* keep empty */
}

GIVEN(R"(a simple data table)")
{
    [[maybe_unused]] auto fixture = context.Get<NordicBleFixture>();
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

STEP(R"(a data table with a single cell)")
{}

STEP(R"(a data table with different fromatting)")
{}

STEP(R"(a data table with an empty cell)")
{}

STEP(R"(a data table with comments and newlines inside)")
{}
