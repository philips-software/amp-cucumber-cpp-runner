#ifndef UTIL_BODY_HPP
#define UTIL_BODY_HPP

#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "gtest/gtest.h"
#include <exception>
#include <gtest/gtest-spi.h>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::util
{

    struct FatalError : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct StepSkipped : std::exception
    {
        StepSkipped(std::string message, std::source_location sourceLocation)
            : message{ std::move(message) }
            , sourceLocation{ sourceLocation }
        {
        }

        std::string message;
        std::source_location sourceLocation;
    };

    struct StepPending : std::exception
    {
        StepPending(std::string message, std::source_location sourceLocation)
            : message{ std::move(message) }
            , sourceLocation{ sourceLocation }
        {
        }

        std::string message;
        std::source_location sourceLocation;
    };

    struct Argument
    {
        std::string converterName;
        cucumber_expression::ConvertFunctionArg converterArgs;
    };

    using ExecuteArgs = std::vector<Argument>;

    struct CucumberResultReporter : public testing::ScopedFakeTestPartResultReporter
    {
        explicit CucumberResultReporter(util::TestStepResult& testStepResult);

        void ReportTestPartResult(const testing::TestPartResult& testPartResult) override;

    private:
        util::TestStepResult& testStepResult;
    };

    struct Body
    {
        virtual ~Body() = default;

        TestStepResult ExecuteAndCatchExceptions(const ExecuteArgs& args = {});

    protected:
        virtual void Execute(const ExecuteArgs& args) = 0;
    };
}

#endif
