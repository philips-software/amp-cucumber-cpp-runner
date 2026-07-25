#ifndef PLUGIN_PLUGIN_EXPORT_HPP
#define PLUGIN_PLUGIN_EXPORT_HPP

#include <cstdint>

#if defined(_WIN32) || defined(__CYGWIN__)
#define CCR_EXPORT __declspec(dllexport)
#else
#define CCR_EXPORT __attribute__((visibility("default")))
#endif

namespace cucumber_cpp::library::plugin
{
    constexpr uint32_t pluginAbiVersion = 1;

    struct PluginHostContext
    {
        uint32_t abiVersion;
        uint32_t structSize;
        void* registration;
        void* stopwatch;
        void* timestampGenerator;
        void* converterMap;
    };

    using CcrRegisterFn = void (*)(PluginHostContext*);
}

#endif
