#include "cucumber_cpp/CucumberCpp.hpp"
#include <gmock/gmock.h>

HOOK_BEFORE_ALL()
{
    Attach("Attachment from BeforeAll hook", "text/plain");
}

WHEN(R"(a step passes)")
{
    // no-op
}

HOOK_AFTER_ALL()
{
    Attach("Attachment from AfterAll hook", "text/plain");
}
