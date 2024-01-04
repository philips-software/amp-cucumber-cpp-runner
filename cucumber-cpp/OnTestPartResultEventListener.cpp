
#include "cucumber-cpp/OnTestPartResultEventListener.hpp"

namespace cucumber_cpp
{
    OnTestPartResultEventListener::OnTestPartResultEventListener()
    {
        testing::UnitTest::GetInstance()->listeners().Append(this);
    }

    OnTestPartResultEventListener::~OnTestPartResultEventListener()
    {
        testing::UnitTest::GetInstance()->listeners().Release(this);
    }

    void OnTestPartResultEventListener::OnTestProgramStart(const testing::UnitTest& unit_test)
    {}

    void OnTestPartResultEventListener::OnTestIterationStart(const testing::UnitTest& unit_test,
        int iteration)
    {}

    void OnTestPartResultEventListener::OnEnvironmentsSetUpStart(const testing::UnitTest& unit_test)
    {}

    void OnTestPartResultEventListener::OnEnvironmentsSetUpEnd(const testing::UnitTest& unit_test)
    {}

    void OnTestPartResultEventListener::OnTestSuiteStart(const testing::TestSuite& /*test_suite*/)
    {}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
    void OnTestPartResultEventListener::OnTestCaseStart(const testing::TestCase& /*test_case*/)
    {}
#endif

    void OnTestPartResultEventListener::OnTestStart(const testing::TestInfo& test_info)
    {}

    void OnTestPartResultEventListener::OnTestDisabled(const testing::TestInfo& /*test_info*/)
    {}

    void OnTestPartResultEventListener::OnTestEnd(const testing::TestInfo& test_info)
    {}

    void OnTestPartResultEventListener::OnTestSuiteEnd(const testing::TestSuite& /*test_suite*/)
    {}

#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
    void OnTestPartResultEventListener::OnTestCaseEnd(const testing::TestCase& /*test_case*/)
    {}
#endif

    void OnTestPartResultEventListener::OnEnvironmentsTearDownStart(const testing::UnitTest& unit_test)
    {}

    void OnTestPartResultEventListener::OnEnvironmentsTearDownEnd(const testing::UnitTest& unit_test)
    {}

    void OnTestPartResultEventListener::OnTestIterationEnd(const testing::UnitTest& unit_test, int iteration)
    {}

    void OnTestPartResultEventListener::OnTestProgramEnd(const testing::UnitTest& unit_test)
    {}
}
