
#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"

namespace cucumber_cpp::library::util
{
    cucumber::messages::TestStepResultStatus TransformTestStepResultStatus(TestStepResultStatus status)
    {
        switch (status)
        {
            case util::TestStepResultStatus::UNKNOWN:
                return cucumber::messages::TestStepResultStatus::UNKNOWN;
            case util::TestStepResultStatus::PASSED:
                return cucumber::messages::TestStepResultStatus::PASSED;
            case util::TestStepResultStatus::SKIPPED:
                return cucumber::messages::TestStepResultStatus::SKIPPED;
            case util::TestStepResultStatus::PENDING:
                return cucumber::messages::TestStepResultStatus::PENDING;
            case util::TestStepResultStatus::UNDEFINED:
                return cucumber::messages::TestStepResultStatus::UNDEFINED;
            case util::TestStepResultStatus::AMBIGUOUS:
                return cucumber::messages::TestStepResultStatus::AMBIGUOUS;
            case util::TestStepResultStatus::FAILED:
                return cucumber::messages::TestStepResultStatus::FAILED;
        }

        return cucumber::messages::TestStepResultStatus::UNKNOWN;
    }
}
