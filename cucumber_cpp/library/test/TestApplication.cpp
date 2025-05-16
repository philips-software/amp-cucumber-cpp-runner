#include "cucumber_cpp/library/Application.hpp"
#include "cucumber_cpp/library/engine/test_helper/TemporaryFile.hpp"
#include <CLI/Error.hpp>
#include <array>
#include <cstddef>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
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

        EXPECT_THAT(Application{}.Run(args.size(), args.data()), testing::Eq(expectedExitCode));

        return testing::internal::GetCapturedStdout();
    }

    TEST_F(TestApplication, RunHelpWithoutArguments)
    {

        const std::array args{ "application" };

        RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::RequiredError));
    }

    TEST_F(TestApplication, RunCommandWithoutArguments)
    {

        const std::array args{ "application", "run" };

        RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::RequiredError));
    }

    TEST_F(TestApplication, RunCommand)
    {

        const std::array args{ "application", "run", "--feature", "./", "--report", "console" };

        RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::Success));
    }

    TEST_F(TestApplication, DryRunCommand)
    {

        const std::array args{ "application", "run", "--feature", "./", "--report", "console", "--dry" };

        RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::Success));
    }

    TEST_F(TestApplication, InvalidArgument)
    {

        const std::array args{ "application", "run", "--feature", "./", "--report", "console", "--doesntexist" };

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

        const std::array args{ "application", "run", "--feature", path.c_str(), "--report", "console", "--dry" };

        std::string stdoutString = RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::Success));

        EXPECT_THAT(stdoutString, testing::HasSubstr("1/1 passed"));
    }

    TEST_F(TestApplication, RunFeatureFile)
    {
        auto tmp = engine::test_helper::TemporaryFile{ "tmpfile.feature" };
        const auto path = tmp.Path().string();

        tmp << "Feature: Test feature\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario1\n"
               "      Given 5 and 5 are equal\n";

        const std::array args{ "application", "run", "--feature", path.c_str(), "--report", "console" };

        std::string stdoutString = RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::Success));

        EXPECT_THAT(stdoutString, testing::HasSubstr("1/1 passed"));
    }

    TEST_F(TestApplication, RunFeatureFileWithError)
    {
        auto tmp = engine::test_helper::TemporaryFile{ "tmpfile.feature" };
        const auto path = tmp.Path().string();

        tmp << "Feature: Test feature\n"
               "  Rule: Test rule\n"
               "    Scenario: Test scenario1\n"
               "      Given 5 and 5 are equal\n"
               "    Scenario: Test scenario2\n"
               "      Given 4 and 5 are equal\n";

        const std::array args{ "application", "run", "--feature", path.c_str(), "--report", "console" };

        std::string stdoutString = RunWithArgs(args, 1);

        EXPECT_THAT(stdoutString, testing::HasSubstr("1/2 passed"));
    }
}
