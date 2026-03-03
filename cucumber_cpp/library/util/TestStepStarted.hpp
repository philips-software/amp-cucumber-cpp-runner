#ifndef UTIL_TEST_STEP_STARTED_HPP
#define UTIL_TEST_STEP_STARTED_HPP

#include <string>

namespace cucumber_cpp::library::util
{
    struct TestStepStarted
    {
        std::string testCaseStartedId;
        std::string testStepId;
    };
}

#endif
