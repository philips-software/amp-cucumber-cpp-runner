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
        virtual ~TestAssertionHandler() = default;

    public:
        virtual void AddAssertionError(const char* file, int line, const std::string& message) = 0;
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

    struct GoogleTestEventListener : testing::EmptyTestEventListener
    {
        explicit GoogleTestEventListener(TestAssertionHandler& testAssertionHandler);

        void OnTestPartResult(const testing::TestPartResult& testPartResult) override;

    private:
        TestAssertionHandler& testAssertionHandler;
    };
}

#endif
