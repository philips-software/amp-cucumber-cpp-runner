#ifndef CUCUMBER_CPP_STEPSMACRO_HPP
#define CUCUMBER_CPP_STEPSMACRO_HPP

#include "cucumber_cpp/library/BodyMacro.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"

#define STEP_(matcher, type, args, fixture) BODY(matcher, type, args, StepRegistry::Register, fixture)

#define STEP_TYPE_(fixture, type, ...)  \
    STEP_(                              \
        BODY_MATCHER(__VA_ARGS__, ""),  \
        type,                           \
        BODY_ARGS(__VA_ARGS__, (), ()), \
        fixture)

#define GIVEN_F(fixture, ...) STEP_TYPE_(fixture, cucumber_cpp::StepType::given, __VA_ARGS__)
#define WHEN_F(fixture, ...) STEP_TYPE_(fixture, cucumber_cpp::StepType::when, __VA_ARGS__)
#define THEN_F(fixture, ...) STEP_TYPE_(fixture, cucumber_cpp::StepType::then, __VA_ARGS__)
#define STEP_F(fixture, ...) STEP_TYPE_(fixture, cucumber_cpp::StepType::any, __VA_ARGS__)

#define GIVEN(...) GIVEN_F(cucumber_cpp::Step, __VA_ARGS__)
#define WHEN(...) WHEN_F(cucumber_cpp::Step, __VA_ARGS__)
#define THEN(...) THEN_F(cucumber_cpp::Step, __VA_ARGS__)
#define STEP(...) STEP_F(cucumber_cpp::Step, __VA_ARGS__)

#endif
