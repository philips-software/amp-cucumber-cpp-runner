#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/Application.hpp"
#include "cucumber_cpp/library/engine/test_helper/TemporaryFile.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <CLI/Error.hpp>
#include <array>
#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>

namespace cucumber_cpp::library
{
    struct TestApplication : testing::Test
    {};

    TEST_F(TestApplication, Construct)
    {
    }

    template<std::size_t S>
    std::string RunWithArgs(const std::array<const char*, S>& args, int expectedExitCode)
    {
        testing::internal::CaptureStdout();

        const auto exitCode = Application{ std::make_shared<ContextStorageFactoryImpl>(), false }.Run(args.size(), args.data());

        const auto capturedStdout = testing::internal::GetCapturedStdout();

        EXPECT_THAT(exitCode, testing::Eq(expectedExitCode));

        return capturedStdout;
    }

    TEST_F(TestApplication, InvalidArgument)
    {

        const std::array args{ "application", "--doesntexist" };

        RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::ExtrasError));
    }

    TEST_F(TestApplication, DryRunFeatureFile)
    {
        auto tmp = engine::test_helper::TemporaryFile{ "tmpfile.feature" };
        const auto path = tmp.Path().string();

        tmp << "Feature: Test feature\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario1\n"
               "      Given 5 and 5 are equal\n";

        const std::array args{ "application", "--format-options", R"({ "summary": {"theme":"plain"} })", "--dry-run", path.c_str() };

        std::string stdoutString = RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::Success));

        EXPECT_THAT(stdoutString, testing::HasSubstr("1 scenarios 1 skipped"));
        EXPECT_THAT(stdoutString, testing::HasSubstr("1 steps 1 skipped"));
    }

    TEST_F(TestApplication, RunFeatureFile)
    {
        auto tmp = engine::test_helper::TemporaryFile{ "tmpfile.feature" };
        const auto path = tmp.Path().string();

        tmp << "Feature: Test feature\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario1\n"
               "      Given 5 and 5 are equal\n";

        const std::array args{ "application", "--format-options", R"({ "summary": {"theme":"plain"} })", path.c_str() };

        std::string stdoutString = RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::Success));

        EXPECT_THAT(stdoutString, testing::HasSubstr("1 scenarios 1 passed"));
        EXPECT_THAT(stdoutString, testing::HasSubstr("1 steps 1 passed"));
    }

    TEST_F(TestApplication, ExposeParameterRegistration)
    {
        EXPECT_THAT(&Application{}.ParameterRegistration(), testing::NotNull());
    }

    TEST_F(TestApplication, UnusedParameters)
    {
        auto tmp = engine::test_helper::TemporaryFile{ "tmpfile.feature" };
        const auto path = tmp.Path().string();

        tmp << "Feature: Test feature\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario1\n"
               "      Given 5 and 5 are equal\n"
               "      And This is a GIVEN step\n"
               "      And This is a WHEN step\n"
               "      And This is a THEN step\n"
               "      And This is a STEP step\n";

        const std::array args{ "application", "--format", "usage", "--format-options", R"({ "usage": {"theme":"plain"} })", "--", path.c_str() };

        std::string stdoutString = RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::Success));

        // used
        EXPECT_THAT(stdoutString, testing::ContainsRegex(R"(This is a GIVEN step\s+\|\s+[0-9]+ms)"));

        // unused
        EXPECT_THAT(stdoutString, testing::ContainsRegex(R"(\^This is a step with a \(\[0-9]\+\)s delay\$\s+\|\s+UNUSED)"));
        EXPECT_THAT(stdoutString, testing::ContainsRegex(R"(I throw an exception\s+\|\s+UNUSED)"));
        EXPECT_THAT(stdoutString, testing::ContainsRegex(R"(Step with cucumber expression syntax \{float\} \{string\} \{int\}\s+\|\s+UNUSED)"));
        EXPECT_THAT(stdoutString, testing::ContainsRegex(R"(An expression that looks like a function func\\\(\{int\}, \{int\}\) should keep its parameters\s+\|\s+UNUSED)"));
    }
}
