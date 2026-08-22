#include "cucumber/messages/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/util/TestStepResultStatus.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResultStatus.hpp"
#include <gmock/gmock.h>

namespace cucumber_cpp::library::util
{
    using testing::Eq;

    TEST(TransformTestStepResultStatus, maps_every_status)
    {
        using From = util::TestStepResultStatus;
        using To = cucumber::messages::TestStepResultStatus;

        EXPECT_THAT(TransformTestStepResultStatus(From::UNKNOWN), Eq(To::UNKNOWN));
        EXPECT_THAT(TransformTestStepResultStatus(From::PASSED), Eq(To::PASSED));
        EXPECT_THAT(TransformTestStepResultStatus(From::SKIPPED), Eq(To::SKIPPED));
        EXPECT_THAT(TransformTestStepResultStatus(From::PENDING), Eq(To::PENDING));
        EXPECT_THAT(TransformTestStepResultStatus(From::UNDEFINED), Eq(To::UNDEFINED));
        EXPECT_THAT(TransformTestStepResultStatus(From::AMBIGUOUS), Eq(To::AMBIGUOUS));
        EXPECT_THAT(TransformTestStepResultStatus(From::FAILED), Eq(To::FAILED));
    }
}
