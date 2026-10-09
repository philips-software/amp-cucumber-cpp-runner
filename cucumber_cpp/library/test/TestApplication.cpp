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

        EXPECT_THAT(stdoutString, testing::HasSubstr("1 scenario (1 skipped)"));
        EXPECT_THAT(stdoutString, testing::HasSubstr("1 step (1 skipped)"));
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

        EXPECT_THAT(stdoutString, testing::HasSubstr("1 scenario (1 passed)"));
        EXPECT_THAT(stdoutString, testing::HasSubstr("1 step (1 passed)"));
    }

    TEST_F(TestApplication, AcceptsSlashPrefixedPathAfterMultiValueOption)
    {
        const std::array args{ "application", "--format-options", R"({ "summary": {"theme":"plain"} })", "--format", "summary", "--", "/nonexistent-ccr-dir" };

        RunWithArgs(args, static_cast<std::underlying_type_t<CLI::ExitCodes>>(CLI::ExitCodes::Success));
    }

    TEST_F(TestApplication, ExposeParameterRegistration)
    {
        EXPECT_THAT(&Application{}.ParameterRegistration(), testing::NotNull());
    }

    TEST_F(TestApplication, RepeatAndRetryAreMutuallyExclusive)
    {
        RunWithArgs(std::array{ "application", "--repeat", "3", "--retry", "2" }, static_cast<int>(CLI::ExitCodes::ExcludesError));
        RunWithArgs(std::array{ "application", "--retry", "0", "--repeat", "1" }, static_cast<int>(CLI::ExitCodes::ExcludesError));
    }

    TEST_F(TestApplication, RepeatRequiresPositiveInteger)
    {
        RunWithArgs(std::array{ "application", "--repeat", "0" }, static_cast<int>(CLI::ExitCodes::ValidationError));
        RunWithArgs(std::array{ "application", "--repeat", "-1" }, static_cast<int>(CLI::ExitCodes::ValidationError));
        RunWithArgs(std::array{ "application", "--repeat", "invalid" }, static_cast<int>(CLI::ExitCodes::ValidationError));
    }

    TEST_F(TestApplication, RepeatTagFilterRequiresRepeat)
    {
        RunWithArgs(std::array{ "application", "--repeat-tag-filter", "@repeat" }, static_cast<int>(CLI::ExitCodes::RequiresError));
    }

    TEST_F(TestApplication, ConfigRepeatConflictsWithCommandLineRetry)
    {
        auto config = engine::test_helper::TemporaryFile{ "repeat.toml" };
        config << "repeat=3\n";
        const auto path = config.Path().string();
        RunWithArgs(std::array{ "application", "--config", path.c_str(), "--retry", "0" }, static_cast<int>(CLI::ExitCodes::ExcludesError));
    }

    TEST_F(TestApplication, ConfigRetryConflictsWithCommandLineRepeat)
    {
        auto config = engine::test_helper::TemporaryFile{ "retry.toml" };
        config << "retry=0\n";
        const auto path = config.Path().string();
        RunWithArgs(std::array{ "application", "--config", path.c_str(), "--repeat", "1" }, static_cast<int>(CLI::ExitCodes::ExcludesError));
    }
}
