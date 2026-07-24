#ifndef PLUGIN_HOOK_LOADER_HPP
#define PLUGIN_HOOK_LOADER_HPP
#ifndef CUCUMBER_CPP_PLUGIN_HOOK_LOADER_HPP
#define CUCUMBER_CPP_PLUGIN_HOOK_LOADER_HPP

#include "cucumber_cpp/library/support/HookRegistry.hpp"

namespace cucumber_cpp::library::plugin
{
    struct HookLoader
    {
        static void Load(support::HookRegistry& hookRegistry);
    };
}

#endif

#endif
