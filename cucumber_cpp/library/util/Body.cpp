
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber/gherkin/demangle.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/ExecuteAndCatchExceptions.hpp"
#include "cucumber_cpp/library/util/NestedTestCaseRunnerError.hpp"
#include "cucumber_cpp/library/util/TestException.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"
#include "fmt/format.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <exception>
#include <filesystem>
#include <gtest/gtest-spi.h>
#include <string>

namespace cucumber_cpp::library::util
{
    CucumberResultReporter::CucumberResultReporter(util::TestStepResult& testStepResult)
        : testing::ScopedFakeTestPartResultReporter{ nullptr }
        , testStepResult{ testStepResult }
    {
    }

    void CucumberResultReporter::ReportTestPartResult(const testing::TestPartResult& testPartResult)
    {
        if (testPartResult.failed())
        {
            testStepResult.status = util::TestStepResultStatus::FAILED;

            auto fileName = std::filesystem::relative(testPartResult.file_name(), std::filesystem::current_path()).string();

            if (testStepResult.message)
                testStepResult.message = fmt::format("{}\n{}:{}: Failure\n{}", testStepResult.message.value(), fileName, testPartResult.line_number(), testPartResult.message());
            else
                testStepResult.message = fmt::format("{}:{}: Failure\n{}", fileName, testPartResult.line_number(), testPartResult.message());
        }

        if (testPartResult.fatally_failed())
            throw FatalError{ testPartResult.message() };
    }

    TestStepResult Body::ExecuteAndCatchExceptions(const ExecuteArgs& args)
    {
        TestStepResult testStepResult{ .status = TestStepResultStatus::PASSED };
        CucumberResultReporter reportListener{ testStepResult };

        try
        {
            Execute(args);

            return testStepResult;
        }
        catch (...)
        {
            return HandleErrors(testStepResult);
        }
    }
}
