#ifndef PLUGIN_STEP_LOADER_HPP
#define PLUGIN_STEP_LOADER_HPP
#ifndef CUCUMBER_CPP_PLUGIN_STEP_LOADER_HPP
#define CUCUMBER_CPP_PLUGIN_STEP_LOADER_HPP

#include "cucumber_cpp/library/support/StepRegistry.hpp"

namespace cucumber_cpp::library::plugin
{
    struct StepLoader
    {
        static void Load(support::StepRegistry& stepRegistry);
    };
}

#endif

#endif
