#include "cucumber_cpp/CucumberCpp.hpp"

GIVEN(R"(an implemented non-pending step)")
{
    // no-op
}

GIVEN(R"(an implemented step that is skipped)")
{
    // no-op
}

GIVEN(R"(an unimplemented pending step)")
{
    Pending();
}
