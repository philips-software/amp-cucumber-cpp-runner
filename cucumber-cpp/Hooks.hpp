#ifndef CUCUMBER_CPP_HOOKS_HPP
#define CUCUMBER_CPP_HOOKS_HPP

#include "cucumber-cpp/BodyMacro.hpp"
#include "cucumber-cpp/HookRegistry.hpp"

#define HOOK_(matcher, type) BODY(matcher, type, (), HookRegistry::Register, cucumber_cpp::HookBase)

#define HOOK_BEFORE_ALL() \
    HOOK_(                \
        "",               \
        cucumber_cpp::HookType::beforeAll)

#define HOOK_AFTER_ALL() \
    HOOK_(               \
        "",              \
        cucumber_cpp::HookType::afterAll)

#define HOOK_BEFORE_FEATURE(...)                     \
    HOOK_(                                           \
        BODY_MATCHER(__VA_ARGS__ __VA_OPT__(, ) ""), \
        cucumber_cpp::HookType::beforeFeature)

#define HOOK_AFTER_FEATURE(...)                      \
    HOOK_(                                           \
        BODY_MATCHER(__VA_ARGS__ __VA_OPT__(, ) ""), \
        cucumber_cpp::HookType::afterFeature)

#define HOOK_BEFORE_SCENARIO(...)                    \
    HOOK_(                                           \
        BODY_MATCHER(__VA_ARGS__ __VA_OPT__(, ) ""), \
        cucumber_cpp::HookType::before)

#define HOOK_AFTER_SCENARIO(...)                     \
    HOOK_(                                           \
        BODY_MATCHER(__VA_ARGS__ __VA_OPT__(, ) ""), \
        cucumber_cpp::HookType::after)

#define HOOK_BEFORE_STEP(...)                        \
    HOOK_(                                           \
        BODY_MATCHER(__VA_ARGS__ __VA_OPT__(, ) ""), \
        cucumber_cpp::HookType::beforeStep)

#define HOOK_AFTER_STEP(...)                         \
    HOOK_(                                           \
        BODY_MATCHER(__VA_ARGS__ __VA_OPT__(, ) ""), \
        cucumber_cpp::HookType::afterStep)

#endif
