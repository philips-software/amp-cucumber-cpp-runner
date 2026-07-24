#ifndef PLUGIN_PLUGIN_EXPORT_HPP
#define PLUGIN_PLUGIN_EXPORT_HPP
#ifndef CUCUMBER_CPP_PLUGIN_EXPORT_HPP
#define CUCUMBER_CPP_PLUGIN_EXPORT_HPP

#if defined(_WIN32) || defined(__CYGWIN__)
#define CCR_EXPORT __declspec(dllexport)
#else
#define CCR_EXPORT __attribute__((visibility("default")))
#endif

using CcrRegisterFn = void (*)();

#endif

#endif
