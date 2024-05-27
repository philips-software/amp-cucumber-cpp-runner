#ifndef DF3DFE0C_FCE8_44FA_9DD4_0A25629A4975
#define DF3DFE0C_FCE8_44FA_9DD4_0A25629A4975

#include <gtest/gtest.h>

namespace cucumber_cpp
{
    struct OnTestPartResultEventListener : testing::TestEventListener
    {
        OnTestPartResultEventListener();
        ~OnTestPartResultEventListener() override;

        void OnTestProgramStart(const testing::UnitTest& unit_test) override;
        void OnTestIterationStart(const testing::UnitTest& unit_test, int iteration) override;
        void OnEnvironmentsSetUpStart(const testing::UnitTest& unit_test) override;
        void OnEnvironmentsSetUpEnd(const testing::UnitTest& unit_test) override;
        void OnTestSuiteStart(const testing::TestSuite& /*test_suite*/) override;

        //  Legacy API is deprecated but still available
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
        void OnTestCaseStart(const testing::TestCase& /*test_case*/) override;
#endif //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_

        void OnTestStart(const testing::TestInfo& test_info) override;
        void OnTestDisabled(const testing::TestInfo& /*test_info*/) override;
        void OnTestEnd(const testing::TestInfo& test_info) override;
        void OnTestSuiteEnd(const testing::TestSuite& /*test_suite*/) override;

//  Legacy API is deprecated but still available
#ifndef GTEST_REMOVE_LEGACY_TEST_CASEAPI_
        void OnTestCaseEnd(const testing::TestCase& /*test_case*/) override;
#endif //  GTEST_REMOVE_LEGACY_TEST_CASEAPI_

        void OnEnvironmentsTearDownStart(const testing::UnitTest& unit_test) override;
        void OnEnvironmentsTearDownEnd(const testing::UnitTest& unit_test) override;
        void OnTestIterationEnd(const testing::UnitTest& unit_test, int iteration) override;
        void OnTestProgramEnd(const testing::UnitTest& unit_test) override;

    private:
        testing::TestEventListeners& listeners{ testing::UnitTest::GetInstance()->listeners() };
    };
}

#endif /* DF3DFE0C_FCE8_44FA_9DD4_0A25629A4975 */
