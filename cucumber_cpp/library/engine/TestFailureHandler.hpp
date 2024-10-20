#ifndef ENGINE_TESTFAILUREHANDLER_HPP
#define ENGINE_TESTFAILUREHANDLER_HPP

#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include "gtest/gtest.h"
#include <gmock/gmock.h>
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

        virtual void AddAssertionError(testing::TestPartResult::Type type, const char* const file, int line, std::string message) = 0;

    private:
        static TestAssertionHandler* instance;
    };

    struct TestAssertionHandlerImpl : TestAssertionHandler
    {
        explicit TestAssertionHandlerImpl(cucumber_cpp::engine::ContextManager& contextManager, report::ReportHandlerV2& reportHandler);
        virtual ~TestAssertionHandlerImpl() = default;

        void AddAssertionError(testing::TestPartResult::Type type, const char* const file, int line, std::string message) override;

    private:
        cucumber_cpp::engine::ContextManager& contextManager;
        report::ReportHandlerV2& reportHandler;
    };

    class CucumberAssertHelper
    {
    public:
        CucumberAssertHelper(testing::TestPartResult::Type type, const char* file, int line,
            const char* message);

        // see testing::internal::AssertHelper
        void operator=(const testing::Message& message) const;

    private:
        // see testing::internal::AssertHelper::AssertHelperData
        struct CucumberAssertHelperData
        {
            CucumberAssertHelperData(testing::TestPartResult::Type t, const char* srcfile, int line_num,
                const char* msg)
                : type(t)
                , file(srcfile)
                , line(line_num)
                , message(msg)
            {}

            testing::TestPartResult::Type const type;
            const char* const file;
            int const line;
            std::string const message;

        private:
            CucumberAssertHelperData(const CucumberAssertHelperData&) = delete;
            CucumberAssertHelperData& operator=(const CucumberAssertHelperData&) = delete;
        };

        std::unique_ptr<const CucumberAssertHelperData> data;

        CucumberAssertHelper(const CucumberAssertHelper&) = delete;
        CucumberAssertHelper& operator=(const CucumberAssertHelper&) = delete;
    };
}

#undef GTEST_MESSAGE_AT_
#define GTEST_MESSAGE_AT_(file, line, message, result_type)                                 \
    cucumber_cpp::library::engine::CucumberAssertHelper(result_type, file, line, message) = \
        ::testing::Message()

#endif
