#ifndef CUCUMBER_CPP_HOOKS_HPP
#define CUCUMBER_CPP_HOOKS_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber_cpp/library/BodyMacro.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"

#define HOOK_(matcher, type) BODY(matcher, type, (), cucumber_cpp::library::support::DefinitionRegistration::Register, cucumber_cpp::library::HookBase)

#define HOOK_BEFORE_ALL(...)                                         \
    HOOK_(                                                           \
        (cucumber_cpp::library::support::GlobalHook{ __VA_ARGS__ }), \
        cucumber_cpp::library::HookType::beforeAll)

#define HOOK_AFTER_ALL(...)                                          \
    HOOK_(                                                           \
        (cucumber_cpp::library::support::GlobalHook{ __VA_ARGS__ }), \
        cucumber_cpp::library::HookType::afterAll)

#define HOOK_BEFORE_FEATURE(...)                               \
    HOOK_(                                                     \
        (cucumber_cpp::library::support::Hook{ __VA_ARGS__ }), \
        cucumber_cpp::library::HookType::beforeFeature)

#define HOOK_AFTER_FEATURE(...)                                \
    HOOK_(                                                     \
        (cucumber_cpp::library::support::Hook{ __VA_ARGS__ }), \
        cucumber_cpp::library::HookType::afterFeature)

#define HOOK_BEFORE_SCENARIO(...)                              \
    HOOK_(                                                     \
        (cucumber_cpp::library::support::Hook{ __VA_ARGS__ }), \
        cucumber_cpp::library::HookType::before)

#define HOOK_AFTER_SCENARIO(...)                               \
    HOOK_(                                                     \
        (cucumber_cpp::library::support::Hook{ __VA_ARGS__ }), \
        cucumber_cpp::library::HookType::after)

#define HOOK_BEFORE_STEP(...)                                  \
    HOOK_(                                                     \
        (cucumber_cpp::library::support::Hook{ __VA_ARGS__ }), \
        cucumber_cpp::library::HookType::beforeStep)

#define HOOK_AFTER_STEP(...)                                   \
    HOOK_(                                                     \
        (cucumber_cpp::library::support::Hook{ __VA_ARGS__ }), \
        cucumber_cpp::library::HookType::afterStep)

#endif
