#include "cucumber_cpp/CucumberCpp.hpp"
#include "fmt/format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

GIVEN("an ambiguous step")
{
    // empty
}

GIVEN("a(n) ambiguous step")
{
    // empty
}

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
    ASSERT_THAT(docString, testing::IsTrue());
    ASSERT_THAT(docString->content, testing::StrEq("Multiline\nDocstring"));
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

GIVEN(R"(a step calls another step with {string})", (const std::string& str))
{
    Step(fmt::format(R"(I store "{}")", str));
}

GIVEN(R"(I store {string})", (const std::string& str))
{
    Step(fmt::format(R"(I store "{}" again)", str));
}

GIVEN(R"(I store {string} again)", (const std::string& str))
{
    context.InsertAt("storedstring", str);
}

THEN(R"(the stored string is {string})", (const std::string& expected))
{
    const auto& stored = context.Get<std::string>("storedstring");
    EXPECT_THAT(stored, testing::StrEq(expected));
}

GIVEN(R"(I attach a link to {string})", (const std::string& url))
{
    Link(url, "title");
}
