#include "cucumber_cpp/library/Hooks.hpp"
#include "cucumber_cpp/library/plugin/PluginExport.hpp"
#include <iostream>

HOOK_BEFORE_SCENARIO()
{
    std::cout << "PLUGIN_HOOK_BEFORE_SCENARIO\n";
}

HOOK_AFTER_SCENARIO()
{
    std::cout << "PLUGIN_HOOK_AFTER_SCENARIO\n";
}
