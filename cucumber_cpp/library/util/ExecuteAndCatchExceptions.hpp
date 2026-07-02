#ifndef UTIL_EXECUTE_AND_CATCH_EXCEPTIONS_HPP
#define UTIL_EXECUTE_AND_CATCH_EXCEPTIONS_HPP

#include "cucumber/gherkin/demangle.hpp"
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/util/Body.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include "cucumber_cpp/library/util/NestedTestCaseRunnerError.hpp"
#include "cucumber_cpp/library/util/TestException.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"
#include "fmt/format.h"
#include <cstddef>
#include <exception>
#include <optional>
#include <string>

namespace cucumber_cpp::library::util
{
    inline void HandleErrors(TestStepResult& testStepResult) noexcept
    {
        try
        {
            throw;
        }
        catch (const util::NestedTestCaseRunnerError& e)
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
    }
}

#endif
