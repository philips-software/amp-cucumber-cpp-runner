#ifndef UTIL_TEST_STEP_RESULT_STATUS_HPP
#define UTIL_TEST_STEP_RESULT_STATUS_HPP

#include <cstdint>

namespace cucumber_cpp::library::util
{
    enum class TestStepResultStatus : std::uint8_t
    {
        UNKNOWN,
        PASSED,
        SKIPPED,
        PENDING,
        UNDEFINED,
        AMBIGUOUS,
        FAILED
    };
}

#endif
