#include "cucumber_cpp/Steps.hpp"
#include <gtest/gtest.h>
#include <string>

GIVEN(R"(^a step$)")
{
    // no-op
}

GIVEN(R"(^a skipped step$)")
{
    Skipped();
}

GIVEN(R"(^a pending step$)")
{
    Pending();
}

GIVEN(R"(^an ambiguous (.*?)$)", (const std::string& arg1))
{
    // no-op
}

GIVEN(R"(^(.*?) ambiguous step$)", (const std::string& arg1))
{
    // no-op
}

GIVEN(R"(^a failing step$)")
{
    FAIL();
}
