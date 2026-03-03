
#include "cucumber/messages/test_step_result_status.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"

namespace cucumber_cpp::library::util
{
    cucumber::messages::test_step_result_status TransformTestStepResultStatus(TestStepResultStatus status)
    {
        switch (status)
        {
            case util::TestStepResultStatus::UNKNOWN:
                return cucumber::messages::test_step_result_status::UNKNOWN;
            case util::TestStepResultStatus::PASSED:
                return cucumber::messages::test_step_result_status::PASSED;
            case util::TestStepResultStatus::SKIPPED:
                return cucumber::messages::test_step_result_status::SKIPPED;
            case util::TestStepResultStatus::PENDING:
                return cucumber::messages::test_step_result_status::PENDING;
            case util::TestStepResultStatus::UNDEFINED:
                return cucumber::messages::test_step_result_status::UNDEFINED;
            case util::TestStepResultStatus::AMBIGUOUS:
                return cucumber::messages::test_step_result_status::AMBIGUOUS;
            case util::TestStepResultStatus::FAILED:
                return cucumber::messages::test_step_result_status::FAILED;
        }

        return cucumber::messages::test_step_result_status::UNKNOWN;
    }
}
