#include "cucumber_cpp/library/Hooks.hpp"
#include <gtest/gtest.h>
#include <string>

namespace cucumber_cpp::library::engine
{
    HOOK_BEFORE_ALL()
    {
        context.InsertAt("hookBeforeAll", std::string{ "hookBeforeAll" });
    }

    HOOK_AFTER_ALL()
    {
        context.InsertAt("hookAfterAll", std::string{ "hookAfterAll" });
    }

    HOOK_BEFORE_FEATURE()
    {
        context.InsertAt("hookBeforeFeature", std::string{ "hookBeforeFeature" });
    }

    HOOK_AFTER_FEATURE()
    {
        context.InsertAt("hookAfterFeature", std::string{ "hookAfterFeature" });
    }

    HOOK_BEFORE_SCENARIO()
    {
        context.InsertAt("hookBeforeScenario", std::string{ "hookBeforeScenario" });
    }

    HOOK_AFTER_SCENARIO()
    {
        context.InsertAt("hookAfterScenario", std::string{ "hookAfterScenario" });
    }

    HOOK_BEFORE_STEP()
    {
        context.InsertAt("hookBeforeStep", std::string{ "hookBeforeStep" });
    }

    HOOK_AFTER_STEP()
    {
        context.InsertAt("hookAfterStep", std::string{ "hookAfterStep" });
    }
}
