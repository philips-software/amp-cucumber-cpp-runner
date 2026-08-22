#include "cucumber/messages/Duration.hpp"
#include "cucumber_cpp/library/util/Duration.hpp"
#include <chrono>
#include <gmock/gmock.h>

namespace cucumber_cpp::library::util
{
    using testing::Eq;

    TEST(Duration, converts_and_adds)
    {
        const auto duration = MillisecondsToDuration(std::chrono::milliseconds{ 1500 });
        EXPECT_THAT(duration.seconds, Eq(1U));
        EXPECT_THAT(DurationToMilliseconds(duration).count(), Eq(1500));
        EXPECT_THAT(DurationToNanoSeconds(duration).count(), Eq(1'500'000'000));

        cucumber::messages::Duration accumulator{ .seconds = 1, .nanos = 800'000'000 };
        accumulator += cucumber::messages::Duration{ .seconds = 0, .nanos = 300'000'000 };
        EXPECT_THAT(accumulator.seconds, Eq(2U));
        EXPECT_THAT(accumulator.nanos, Eq(100'000'000U));

        const auto sum = cucumber::messages::Duration{ .seconds = 1, .nanos = 0 } + cucumber::messages::Duration{ .seconds = 2, .nanos = 0 };
        EXPECT_THAT(sum.seconds, Eq(3U));
    }
}
