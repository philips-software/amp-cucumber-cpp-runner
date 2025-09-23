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
    namespace
    {
        std::string AppendUserMessage(const std::string& gtest_msg, const testing::Message& user_msg)
        {
            std::string user_msg_string = user_msg.GetString();

            if (user_msg_string.empty())
                return gtest_msg;

            if (gtest_msg.empty())
                return user_msg_string;

            return gtest_msg + "\n" + user_msg_string;
        }
    }

    TestAssertionHandler* TestAssertionHandler::instance{ nullptr }; // NOLINT

    TestAssertionHandler::TestAssertionHandler()
    {
        TestAssertionHandler::instance = this;
    }

    TestAssertionHandler::~TestAssertionHandler()
    {
        TestAssertionHandler::instance = nullptr;
    }

    TestAssertionHandler& TestAssertionHandler::Instance()
    {
        return *instance;
    }

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

    // CucumberAssertHelper::CucumberAssertHelper(testing::TestPartResult::Type type, const char* file, int line, const char* message)
    //     : data(std::make_unique<CucumberAssertHelperData>(type, file, line, message))
    // {}

    // void CucumberAssertHelper::operator=(const testing::Message& message) const // NOLINT
    // {
    //     TestAssertionHandler::Instance().AddAssertionError(data->file, data->line, AppendUserMessage(data->message, message));
    // }

    GoogleTestEventListener::GoogleTestEventListener(TestAssertionHandler& testAssertionHandler)
        : testAssertionHandler(testAssertionHandler)
    {}

    void GoogleTestEventListener::OnTestPartResult(const testing::TestPartResult& testPartResult)
    {
        if (testPartResult.failed())
        {
            TestAssertionHandler::Instance().AddAssertionError(
                testPartResult.file_name() != nullptr ? testPartResult.file_name() : "",
                testPartResult.line_number(),
                testPartResult.message());
        }
    }
}
