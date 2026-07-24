#include "cucumber_cpp/library/plugin/HookLoader.hpp"
#include "cucumber_cpp/library/support/HookRegistry.hpp"

namespace cucumber_cpp::library::plugin
{
    void HookLoader::Load(support::HookRegistry& hookRegistry)
    {
        hookRegistry.LoadHooks();
    }
}
