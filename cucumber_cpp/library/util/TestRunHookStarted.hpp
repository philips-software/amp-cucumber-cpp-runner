#ifndef UTIL_TEST_RUN_HOOK_STARTED_HPP
#define UTIL_TEST_RUN_HOOK_STARTED_HPP

#include <string>

namespace cucumber_cpp::library::util
{
    struct TestRunHookStarted
    {
        std::string testRunStartedId;
    };
}

#endif
