#include "cucumber_cpp/library/support/Body.hpp"
#include "cucumber/gherkin/demangle.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/runtime/NestedTestCaseRunner.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "fmt/format.h"
#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"
#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <string>

namespace cucumber_cpp::library::support
{
    struct CucumberResultReporter : public testing::ScopedFakeTestPartResultReporter
    {
        explicit CucumberResultReporter(TestStepResult& testStepResult)
            : testing::ScopedFakeTestPartResultReporter{ nullptr }
            , testStepResult{ testStepResult }
        {
        }

        void ReportTestPartResult(const testing::TestPartResult& testPartResult) override
        {
            if (testPartResult.failed())
            {
                testStepResult.status = TestStepResultStatus::FAILED;

                auto fileName = std::filesystem::relative(testPartResult.file_name(), std::filesystem::current_path()).string();

                if (testStepResult.message)
                    testStepResult.message = fmt::format("{}\n{}:{}: Failure\n{}", testStepResult.message.value(), fileName, testPartResult.line_number(), testPartResult.message());
                else
                    testStepResult.message = fmt::format("{}:{}: Failure\n{}", fileName, testPartResult.line_number(), testPartResult.message());
            }

            if (testPartResult.fatally_failed())
                throw FatalError{ testPartResult.message() };
        }

    private:
        TestStepResult& testStepResult;
    };

    TestStepResult Body::ExecuteAndCatchExceptions(const ExecuteArgs& args)
    {
        TestStepResult testStepResult{ .status = TestStepResultStatus::PASSED };
        CucumberResultReporter reportListener{ testStepResult };

        const auto startTime = util::Stopwatch::Instance().Start();
        try
        {
            Execute(args);
        }
        catch (const runtime::NestedTestCaseRunnerError& e)
        {
            testStepResult.status = TestStepResultStatus::FAILED;

            if (e.status.status != cucumber::messages::test_step_result_status::PASSED)
            {
                const auto offset = std::string(e.nesting, ' ');

                if (e.status.message.has_value())
                    testStepResult.message = fmt::format(R"({0} {1} nested step: "* {2}")"
                                                         "\n{0} {3}",
                        offset,
                        cucumber::messages::to_string(e.status.status),
                        e.text,
                        e.status.message.value());
                else
                    testStepResult.message = fmt::format(R"({0} {1} nested step: "* {2}")",
                        offset,
                        cucumber::messages::to_string(e.status.status),
                        e.text);
            }
        }
        catch (const StepSkipped& e)
        {
            testStepResult.status = TestStepResultStatus::SKIPPED;
            if (!e.message.empty())
                testStepResult.message = e.message;
        }
        catch (const StepPending& e)
        {
            testStepResult.status = TestStepResultStatus::PENDING;
            if (!e.message.empty())
                testStepResult.message = e.message;
        }
        catch ([[maybe_unused]] const FatalError& error)
        {
            testStepResult.status = TestStepResultStatus::FAILED;
        }
        catch (std::exception& e)
        {
            testStepResult.status = TestStepResultStatus::FAILED;
            testStepResult.exception = TestException{
                .type = cucumber::gherkin::detail::demangle(typeid(e).name()).get(),
                .message = e.what(),
            };
        }
        catch (...)
        {
            testStepResult.status = TestStepResultStatus::FAILED;
            testStepResult.exception = TestException{
                .type = "unknown",
                .message = "unknown exception",
            };
        }

        auto nanoseconds = util::Stopwatch::Instance().Duration(startTime);
        static constexpr std::size_t nanosecondsPerSecond = 1e9;
        testStepResult.duration = {
            .seconds = nanoseconds.count() / nanosecondsPerSecond,
            .nanos = nanoseconds.count() % nanosecondsPerSecond,
        };

        return testStepResult;
    }
}
