#include "acceptance/util/Subprocess.hpp"
#include <filesystem>
#include <gmock/gmock.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace acceptance::util
{
#ifndef SUBPROCESS_TEST_CHILD
#error "SUBPROCESS_TEST_CHILD must be defined"
#endif

    namespace
    {
        ProcessResult RunChild(const std::vector<std::string>& arguments)
        {
            return RunProcess(std::filesystem::path{ SUBPROCESS_TEST_CHILD }, arguments);
        }
    }

    TEST(Subprocess, captures_stdout_and_stderr_separately)
    {
        const auto result = RunChild({ "--stdout", "alpha", "--stderr", "beta", "--exit", "0" });

        EXPECT_THAT(result.exitCode, testing::Eq(0));
        EXPECT_THAT(result.standardOutput, testing::StrEq("alpha"));
        EXPECT_THAT(result.standardError, testing::StrEq("beta"));
    }

    TEST(Subprocess, captures_newlines_without_merging_streams)
    {
        const auto result = RunChild({ "--stdout", "line1\nline2", "--stderr", "err1\nerr2", "--exit", "0" });

        EXPECT_THAT(result.exitCode, testing::Eq(0));
        EXPECT_THAT(result.standardOutput, testing::StrEq("line1\nline2"));
        EXPECT_THAT(result.standardError, testing::StrEq("err1\nerr2"));
    }

    TEST(Subprocess, propagates_non_zero_exit_code)
    {
        const auto result = RunChild({ "--stdout", "ok", "--exit", "7" });

        EXPECT_THAT(result.exitCode, testing::Eq(7));
        EXPECT_THAT(result.standardOutput, testing::StrEq("ok"));
        EXPECT_THAT(result.standardError, testing::IsEmpty());
    }

    TEST(Subprocess, handles_empty_output)
    {
        const auto result = RunChild({ "--exit", "0" });

        EXPECT_THAT(result.exitCode, testing::Eq(0));
        EXPECT_THAT(result.standardOutput, testing::IsEmpty());
        EXPECT_THAT(result.standardError, testing::IsEmpty());
    }

    TEST(Subprocess, drains_large_stdout_and_stderr_without_deadlock)
    {
        const auto result = RunChild({ "--out-bytes", "131072", "--err-bytes", "131072", "--exit", "0" });

        EXPECT_THAT(result.exitCode, testing::Eq(0));
        EXPECT_THAT(result.standardOutput.size(), testing::Eq(131072U));
        EXPECT_THAT(result.standardError.size(), testing::Eq(131072U));
        EXPECT_THAT(result.standardOutput.front(), testing::Eq('O'));
        EXPECT_THAT(result.standardError.front(), testing::Eq('E'));
    }

    TEST(Subprocess, throws_for_missing_executable)
    {
        const auto missingPath = std::filesystem::temp_directory_path() / "cucumber_cpp_missing_executable";

        EXPECT_THROW((void)RunProcess(missingPath, {}), std::runtime_error);
    }
}
