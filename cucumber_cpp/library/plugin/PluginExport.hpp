#ifndef PLUGIN_PLUGIN_EXPORT_HPP
#define PLUGIN_PLUGIN_EXPORT_HPP

#if defined(_WIN32) || defined(__CYGWIN__)
#define CCR_EXPORT __declspec(dllexport)
#else
#define CCR_EXPORT __attribute__((visibility("default")))
#endif

namespace cucumber_cpp::library::plugin
{
    struct PluginHostContext
    {
        void* registration;
        void* stopwatch;
        void* timestampGenerator;
        void* converterMap;
    };
}

using CcrRegisterFn = void (*)(cucumber_cpp::library::plugin::PluginHostContext*);

#endif
