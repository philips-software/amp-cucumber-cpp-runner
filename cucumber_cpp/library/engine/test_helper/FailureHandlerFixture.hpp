#ifndef TEST_HELPER_FAILURE_HANDLER_FIXTURE_HPP
#define TEST_HELPER_FAILURE_HANDLER_FIXTURE_HPP

#include "cucumber_cpp/CucumberCpp.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FailureHandler.hpp"

namespace cucumber_cpp::library::engine::test_helper
{
    struct FailureHandlerFixture
    {
        explicit FailureHandlerFixture(ContextManager& contextManager);
        ~FailureHandlerFixture();

        ContextManager& contextManager;

        report::ReportForwarderImpl reportHandler{ contextManager };
        TestAssertionHandlerImpl testAssertionHandler{ contextManager, reportHandler };
        GoogleTestEventListener googleTestEventListener{ testAssertionHandler };
    };

}

#endif
