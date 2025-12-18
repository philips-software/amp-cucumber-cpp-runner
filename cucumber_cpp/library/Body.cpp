#include "cucumber_cpp/library/Body.hpp"
#include "cucumber/messages/exception.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/engine/ExecutionContext.hpp"
#include "cucumber_cpp/library/support/Duration.hpp"
#include "gtest/gtest.h"
#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <format>
#include <gtest/gtest-spi.h>
#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <src/gtest-internal-inl.h>

namespace cucumber_cpp::library
{
    struct CucumberResultReporter : public testing::ScopedFakeTestPartResultReporter
    {
        explicit CucumberResultReporter(cucumber::messages::test_step_result& testStepResult)
            : testing::ScopedFakeTestPartResultReporter{ nullptr }
            , testStepResult{ testStepResult }
        {
        }

        void ReportTestPartResult(const testing::TestPartResult& testPartResult)
        {
            if (testPartResult.failed())
            {
                testStepResult.status = cucumber::messages::test_step_result_status::FAILED;

                auto fileName = std::filesystem::relative(testPartResult.file_name(), std::filesystem::current_path()).string();

                if (testStepResult.message)
                    testStepResult.message = std::format("{}\n{}:{}: Failure\n{}", testStepResult.message.value(), fileName, testPartResult.line_number(), testPartResult.message());
                else
                    testStepResult.message = std::format("{}:{}: Failure\n{}", fileName, testPartResult.line_number(), testPartResult.message());
            }

            if (testPartResult.fatally_failed())
                throw FatalError{ testPartResult.message() };
        }

    private:
        cucumber::messages::test_step_result& testStepResult;
    };

    cucumber::messages::test_step_result Body::ExecuteAndCatchExceptions(const ExecuteArgs& args)
    {
        cucumber::messages::test_step_result testStepResult{ .status = cucumber::messages::test_step_result_status::PASSED };
        CucumberResultReporter reportListener{ testStepResult };

        try
        {
            support::Stopwatch::Instance().Start();
            Execute(args);
        }
        catch (const engine::StepSkipped& e)
        {
            testStepResult.status = cucumber::messages::test_step_result_status::SKIPPED;
            if (!e.message.empty())
                testStepResult.message = e.message;
        }
        catch (const engine::StepPending& e)
        {
            testStepResult.status = cucumber::messages::test_step_result_status::PENDING;
            if (!e.message.empty())
                testStepResult.message = e.message;
        }
        catch (const FatalError& error)
        {
            testStepResult.status = cucumber::messages::test_step_result_status::FAILED;
        }
        catch (std::exception& e)
        {
            testStepResult.status = cucumber::messages::test_step_result_status::FAILED;
            testStepResult.exception = cucumber::messages::exception{
                .type = typeid(e).name(),
                .message = e.what(),
            };
        }
        catch (...)
        {
            testStepResult.status = cucumber::messages::test_step_result_status::FAILED;
            testStepResult.exception = cucumber::messages::exception{
                .type = "unknown",
                .message = "unknown exception",
            };
        }

        auto nanoseconds = support::Stopwatch::Instance().Duration();
        static constexpr std::size_t nanosecondsPerSecond = 1e9;
        testStepResult.duration = {
            .seconds = static_cast<std::size_t>(nanoseconds.count() / static_cast<std::size_t>(nanosecondsPerSecond)),
            .nanos = static_cast<std::size_t>(nanoseconds.count() % static_cast<std::size_t>(nanosecondsPerSecond)),
        };

        return testStepResult;
    }
}
