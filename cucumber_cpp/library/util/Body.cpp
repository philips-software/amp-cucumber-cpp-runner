
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/ExecuteAndCatchExceptions.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"
#include "fmt/format.h"
#include "gtest/gtest.h"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <gtest/gtest-spi.h>
#include <memory>

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

    TestStepResult ConstructAndExecute(const std::function<std::unique_ptr<Body>()>& bodyFactory, const ExecuteArgs& args)
    {
        const auto startTime = Stopwatch::Instance().Start();
        TestStepResult testStepResult{ .status = TestStepResultStatus::PASSED };
        CucumberResultReporter reportListener{ testStepResult };

        try
        {
            bodyFactory()->Execute(args);
        }
        catch (...)
        {
            HandleErrors(testStepResult);
        }

        auto nanoseconds = Stopwatch::Instance().Duration(startTime);
        static constexpr std::size_t nanosecondsPerSecond = 1e9;
        testStepResult.duration = {
            .seconds = nanoseconds.count() / nanosecondsPerSecond,
            .nanos = nanoseconds.count() % nanosecondsPerSecond,
        };

        return testStepResult;
    }
}
