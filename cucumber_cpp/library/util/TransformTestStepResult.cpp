#include "cucumber_cpp/library/util/TransformTestStepResult.hpp"
#include "cucumber/messages/duration.hpp"
#include "cucumber/messages/exception.hpp"
#include "cucumber/messages/test_step_result.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResultStatus.hpp"
#include <optional>

namespace cucumber_cpp::library::util
{
    cucumber::messages::test_step_result TransformTestStepResult(util::TestStepResult result)
    {
        return {
            .duration = cucumber::messages::duration{
                .seconds = result.duration.seconds,
                .nanos = result.duration.nanos,
            },
            .message = result.message,
            .status = util::TransformTestStepResultStatus(result.status),
            .exception = result.exception.has_value() ? std::make_optional<cucumber::messages::exception>(result.exception->type, result.exception->message) : std::nullopt,
        };
    }
}
