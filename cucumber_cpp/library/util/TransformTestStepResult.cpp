#include "cucumber_cpp/library/util/TransformTestStepResult.hpp"
#include "cucumber/messages/Duration.hpp"
#include "cucumber/messages/Exception.hpp"
#include "cucumber/messages/TestStepResult.hpp"
#include "cucumber_cpp/library/util/TestStepResult.hpp"
#include "cucumber_cpp/library/util/TransformTestStepResultStatus.hpp"
#include <memory>
#include <optional>

namespace cucumber_cpp::library::util
{
    cucumber::messages::TestStepResult TransformTestStepResult(util::TestStepResult result)
    {
        return {
            .duration = std::make_shared<cucumber::messages::Duration>(cucumber::messages::Duration{
                .seconds = result.duration.seconds,
                .nanos = result.duration.nanos,
            }),
            .message = result.message,
            .status = util::TransformTestStepResultStatus(result.status),
            .exception = result.exception.has_value() ? std::make_optional(std::make_shared<cucumber::messages::Exception>(cucumber::messages::Exception{ .type = result.exception->type, .message = result.exception->message })) : std::nullopt,
        };
    }
}
