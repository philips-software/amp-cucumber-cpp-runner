
#include "cucumber-cpp/OnTestPartResultEventListener.hpp"
#include <gtest/gtest.h>
#include <memory>

namespace cucumber_cpp
{
    OnTestPartResultEventListener::OnTestPartResultEventListener()
    {
        if (auto* defaultPrinter = listeners.default_result_printer(); defaultPrinter != nullptr)
            std::unique_ptr<TestEventListener>{ listeners.Release(defaultPrinter) };

        listeners.Append(this);
    }

    OnTestPartResultEventListener::~OnTestPartResultEventListener()
    {
        listeners.Release(this);
    }

    void OnTestPartResultEventListener::OnTestProgramStart(const testing::UnitTest& unit_test)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnTestIterationStart(const testing::UnitTest& unit_test, int iteration)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnEnvironmentsSetUpStart(const testing::UnitTest& unit_test)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnEnvironmentsSetUpEnd(const testing::UnitTest& unit_test)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnTestSuiteStart(const testing::TestSuite& /*test_suite*/)
    {
        // no implementation
    }

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
    void OnTestPartResultEventListener::OnTestCaseStart(const testing::TestCase& /*test_case*/)
    {
        // no implementation
    }
#endif

    void OnTestPartResultEventListener::OnTestStart(const testing::TestInfo& test_info)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnTestDisabled(const testing::TestInfo& /*test_info*/)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnTestEnd(const testing::TestInfo& test_info)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnTestSuiteEnd(const testing::TestSuite& /*test_suite*/)
    {
        // no implementation
    }

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
    void OnTestPartResultEventListener::OnTestCaseEnd(const testing::TestCase& /*test_case*/)
    {
        // no implementation
    }
#endif

    void OnTestPartResultEventListener::OnEnvironmentsTearDownStart(const testing::UnitTest& unit_test)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnEnvironmentsTearDownEnd(const testing::UnitTest& unit_test)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnTestIterationEnd(const testing::UnitTest& unit_test, int iteration)
    {
        // no implementation
    }

    void OnTestPartResultEventListener::OnTestProgramEnd(const testing::UnitTest& unit_test)
    {
        // no implementation
    }
}
