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
        cucumber::messages::Duration duration;
        duration.seconds = result.duration.seconds;
        duration.nanos = result.duration.nanos;

        cucumber::messages::TestStepResult testStepResult;
        testStepResult.duration = duration;
        testStepResult.message = result.message;
        testStepResult.status = util::TransformTestStepResultStatus(result.status);
        if (result.exception.has_value())
        {
            cucumber::messages::Exception exception;
            exception.type = result.exception->type;
            exception.message = result.exception->message;
            testStepResult.exception = exception;
        }
        return testStepResult;
    }
}
