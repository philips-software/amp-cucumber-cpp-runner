#ifndef UTIL_BODY_HPP
#define UTIL_BODY_HPP

#include "cucumber_cpp/library/cucumber_expression/ParameterRegistry.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "gtest/gtest-spi.h"
#include "gtest/gtest.h"
#include <exception>
#include <functional>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::util
{
    struct CucumberResultReporter : public testing::ScopedFakeTestPartResultReporter
    {
        explicit CucumberResultReporter(util::TestStepResult& testStepResult);

        void ReportTestPartResult(const testing::TestPartResult& testPartResult) override;

    private:
        util::TestStepResult& testStepResult;
    };

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

    struct Body;

    using BodyFactory = std::function<std::unique_ptr<Body>(TestStepResult&)>;
    TestStepResult ConstructAndExecute(const BodyFactory& bodyFactory, const ExecuteArgs& args = {});

    struct Body
    {
        explicit Body(TestStepResult& testStepResult);
        virtual ~Body() = default;

    private:
        virtual void Execute(const ExecuteArgs& args) = 0;

        friend TestStepResult ConstructAndExecute(const BodyFactory& bodyFactory, const ExecuteArgs& args);

        CucumberResultReporter reportListener;
    };
}

#endif
