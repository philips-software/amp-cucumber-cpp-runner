#include "cucumber_cpp/Steps.hpp"

namespace
{
    auto executionCount = 0;
    auto taggedExecutionCount = 0;
}

GIVEN(R"(a step that counts executions)")
{
    executionCount++;
}

GIVEN(R"(a step that counts tagged executions)")
{
    taggedExecutionCount++;
}

