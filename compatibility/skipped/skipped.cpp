#include "cucumber_cpp/CucumberCpp.hpp"

GIVEN(R"(a step that does not skip)")
{
    // no-op
}

WHEN(R"(a step that is skipped)")
{
    // no-op
}

THEN(R"(I skip a step)")
{
    Skipped();
}
