#ifndef CUCUMBER_CPP_STEPSMACRO_HPP
#define CUCUMBER_CPP_STEPSMACRO_HPP

#include "cucumber-cpp/BodyMacro.hpp"
#include "cucumber-cpp/StepRegistry.hpp"

#define STEP_(matcher, type, args) BODY(matcher, type, args, StepRegistry::Register, StepBase)

#define GIVEN(...)                     \
    STEP_(                             \
        BODY_MATCHER(__VA_ARGS__, ""), \
        cucumber_cpp::StepType::given, \
        BODY_ARGS(__VA_ARGS__, (), ()))

#define WHEN(...)                      \
    STEP_(                             \
        BODY_MATCHER(__VA_ARGS__, ""), \
        cucumber_cpp::StepType::when,  \
        BODY_ARGS(__VA_ARGS__, (), ()))

#define THEN(...)                      \
    STEP_(                             \
        BODY_MATCHER(__VA_ARGS__, ""), \
        cucumber_cpp::StepType::then,  \
        BODY_ARGS(__VA_ARGS__, (), ()))

#define STEP(...)                      \
    STEP_(                             \
        BODY_MATCHER(__VA_ARGS__, ""), \
        cucumber_cpp::StepType::any,   \
        BODY_ARGS(__VA_ARGS__, (), ()))

#endif
