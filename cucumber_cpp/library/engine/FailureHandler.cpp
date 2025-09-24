#include "cucumber_cpp/library/engine/FailureHandler.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <memory>
#include <string>

namespace cucumber_cpp::library::engine
{
    TestAssertionHandlerImpl::TestAssertionHandlerImpl(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler)
        : contextManager{ contextManager }
        , reportHandler{ reportHandler }
    {}

    void TestAssertionHandlerImpl::AddAssertionError(const char* file, int line, const std::string& message)
    {
        std::filesystem::path relativeFilePath = std::filesystem::relative(file);

        contextManager.CurrentContext().ExecutionStatus(cucumber_cpp::library::engine::Result::failed);

        if (auto* stepContext = contextManager.CurrentStepContext(); stepContext != nullptr)
            stepContext->ExecutionStatus(cucumber_cpp::library::engine::Result::failed);

        reportHandler.Failure(message, relativeFilePath, line);
    }

    GoogleTestEventListener::GoogleTestEventListener(TestAssertionHandler& testAssertionHandler)
        : testAssertionHandler(testAssertionHandler)
    {}

    void GoogleTestEventListener::OnTestPartResult(const testing::TestPartResult& testPartResult)
    {
        if (testPartResult.failed())
        {
            testAssertionHandler.AddAssertionError(
                testPartResult.file_name() != nullptr ? testPartResult.file_name() : "",
                testPartResult.line_number(),
                testPartResult.message());
        }
    }
}
