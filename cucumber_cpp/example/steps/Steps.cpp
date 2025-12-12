#include "cucumber_cpp/library/Steps.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <iostream>
#include <string>

GIVEN(R"(a background step)")
{
}

GIVEN(R"(a simple data table)")
{
    // std::cout << "row0.col0: " << table[0][0].As<std::string>() << "\n";
    // std::cout << "row0.col1: " << table[0][1].As<std::string>() << "\n";

    // std::cout << "row1.col0: " << table[1][0].As<std::string>() << "\n";
    // std::cout << "row1.col1: " << table[1][1].As<std::string>() << "\n";
}

GIVEN(R"(there are {int} cucumbers)", (std::int32_t num))
{
    context.InsertAt("cucumbers_before", num);
}

STEP(R"(I eat {int} cucumbers)", (std::int32_t num))
{
    context.InsertAt("cucumbers_eaten", num);
}

STEP("expect and assert")
{
    EXPECT_THAT(false, testing::Eq(true));
    ASSERT_THAT(true, testing::Eq(false));
}

THEN(R"(^I should have ([0-9]+) cucumbers$)", (std::int32_t num))
{
    const auto& before = context.Get<std::int32_t>("cucumbers_before");
    const auto& eaten = context.Get<std::int32_t>("cucumbers_eaten");

    const auto actual = before - eaten;

    ASSERT_THAT(actual, testing::Eq(num));
}

THEN(R"(^I should have ([0-9]+) cucumbers left$)", (std::int32_t num))
{
    const auto& before = context.Get<std::int32_t>("cucumbers_before");
    const auto& eaten = context.Get<std::int32_t>("cucumbers_eaten");

    const auto actual = before - eaten;

    ASSERT_THAT(actual, testing::Eq(num));
}

STEP(R"(a data table with a single cell)")
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

STEP(R"(^a step$)")
{
    context.EmplaceAt<std::string>("substep", "was executed");
}

STEP(R"(^a step calls another step$)")
{
    Given(R"(a step)");
}

STEP(R"(^the called step is executed$)")
{
    ASSERT_THAT(context.Contains("substep"), testing::IsTrue());
    ASSERT_THAT(context.Get<std::string>("substep"), testing::Eq("was executed"));
}

STEP("this step should be skipped")
{
    /* no body, example only */
}

STEP(R"(a step stores the value at row {int} and column {int} from the table:)", (std::int32_t row, std::int32_t column))
{
    context.EmplaceAt<std::string>("cell", table.value()[row].cells[column].value);
}

STEP(R"(the value should be {string})", (const std::string& expected_value))
{
    const auto& actual = context.Get<std::string>("cell");

    ASSERT_THAT(actual, testing::StrEq(expected_value));
}
