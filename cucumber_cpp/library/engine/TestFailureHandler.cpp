#include "cucumber_cpp/library/engine/TestFailureHandler.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <format>
#include <iomanip>
#include <memory>
#include <string>

namespace cucumber_cpp::library::engine
{
    namespace
    {
        std::string AppendUserMessage(const std::string& gtest_msg, const testing::Message& user_msg)
        {
            const std::string user_msg_string = user_msg.GetString();

            if (user_msg_string.empty())
                return gtest_msg;

            if (gtest_msg.empty())
                return user_msg_string;

            return gtest_msg + "\n" + user_msg_string;
        }
    }

    TestAssertionHandler* TestAssertionHandler::instance{ nullptr };

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

    TestAssertionHandlerImpl::TestAssertionHandlerImpl(cucumber_cpp::engine::ContextManager& contextManager, report::ReportHandlerV2& reportHandler)
        : contextManager{ contextManager }
        , reportHandler{ reportHandler }
    {}

    void TestAssertionHandlerImpl::AddAssertionError(testing::TestPartResult::Type type, const char* const file, int line, std::string message)
    {
        std::filesystem::path relativeFilePath = std::filesystem::relative(file);

        contextManager.StepContext().ExecutionStatus(cucumber_cpp::engine::Result::failed);

        reportHandler.Failure(message, relativeFilePath, line);
    }

    CucumberAssertHelper::CucumberAssertHelper(testing::TestPartResult::Type type, const char* file,
        int line, const char* message)
        : data(std::make_unique<CucumberAssertHelperData>(type, file, line, message))
    {}

    void CucumberAssertHelper::operator=(const testing::Message& message) const
    {
        TestAssertionHandler::Instance().AddAssertionError(data->type, data->file, data->line, AppendUserMessage(data->message, message));
    }
}
