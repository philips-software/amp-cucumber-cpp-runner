#ifndef ENGINE_FAILUREHANDLER_HPP
#define ENGINE_FAILUREHANDLER_HPP

#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace cucumber_cpp::library::engine
{
    struct TestAssertionHandler
    {
    protected:
        TestAssertionHandler();
        ~TestAssertionHandler();

    public:
        TestAssertionHandler(const TestAssertionHandler&) = delete;

        static TestAssertionHandler& Instance();

        virtual void AddAssertionError(const char* file, int line, const std::string& message) = 0;

    private:
        static TestAssertionHandler* instance; // NOLINT
    };

    struct TestAssertionHandlerImpl : TestAssertionHandler
    {
        explicit TestAssertionHandlerImpl(cucumber_cpp::library::engine::ContextManager& contextManager, report::ReportForwarder& reportHandler);
        virtual ~TestAssertionHandlerImpl() = default;

        void AddAssertionError(const char* file, int line, const std::string& message) override;

    private:
        cucumber_cpp::library::engine::ContextManager& contextManager;
        report::ReportForwarder& reportHandler;
    };

    // class CucumberAssertHelper
    // {
    // public:
    //     CucumberAssertHelper(testing::TestPartResult::Type type, const char* file, int line,
    //         const char* message);

    //     CucumberAssertHelper(const CucumberAssertHelper&) = delete;
    //     CucumberAssertHelper& operator=(const CucumberAssertHelper&) = delete;

    //     // see testing::internal::AssertHelper
    //     void operator=(const testing::Message& message) const; // NOLINT

    // private:
    //     // see testing::internal::AssertHelper::AssertHelperData
    //     struct CucumberAssertHelperData
    //     {
    //         CucumberAssertHelperData(testing::TestPartResult::Type t, const char* srcfile, int line_num,
    //             const char* msg)
    //             : type(t)
    //             , file(srcfile)
    //             , line(line_num)
    //             , message(msg)
    //         {}

    //         const testing::TestPartResult::Type type;
    //         const char* const file;
    //         const int line;
    //         const std::string message;

    //         CucumberAssertHelperData(const CucumberAssertHelperData&) = delete;
    //         CucumberAssertHelperData& operator=(const CucumberAssertHelperData&) = delete;
    //     };

    //     std::unique_ptr<const CucumberAssertHelperData> data;
    // };

    struct GoogleTestEventListener : testing::EmptyTestEventListener
    {
        explicit GoogleTestEventListener(TestAssertionHandler& testAssertionHandler);

        void OnTestPartResult(const testing::TestPartResult& testPartResult) override;

    private:
        TestAssertionHandler& testAssertionHandler;
    };
}

#endif
