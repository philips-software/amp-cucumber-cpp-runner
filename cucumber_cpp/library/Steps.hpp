#ifndef CUCUMBER_CPP_STEPSMACRO_HPP
#define CUCUMBER_CPP_STEPSMACRO_HPP

// IWYU pragma: private, include "cucumber_cpp/CucumberCpp.hpp"
// IWYU pragma: friend cucumber_cpp/.*

#include "cucumber_cpp/library/BodyMacro.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/Step.hpp"
#include "cucumber_cpp/library/support/SupportCodeLibrary.hpp"

#define STEP_(matcher, type, args, fixture) BODY(matcher, type, args, cucumber_cpp::library::support::DefinitionRegistration::Register, fixture)

#define STEP_TYPE_(fixture, type, ...)  \
    STEP_(                              \
        BODY_MATCHER(__VA_ARGS__, ""),  \
        type,                           \
        BODY_ARGS(__VA_ARGS__, (), ()), \
        fixture)

#define GIVEN_F(fixture, ...) STEP_TYPE_(fixture, cucumber_cpp::library::engine::StepType::given, __VA_ARGS__)
#define WHEN_F(fixture, ...) STEP_TYPE_(fixture, cucumber_cpp::library::engine::StepType::when, __VA_ARGS__)
#define THEN_F(fixture, ...) STEP_TYPE_(fixture, cucumber_cpp::library::engine::StepType::then, __VA_ARGS__)
#define STEP_F(fixture, ...) STEP_TYPE_(fixture, cucumber_cpp::library::engine::StepType::any, __VA_ARGS__)

#define GIVEN(...) GIVEN_F(cucumber_cpp::library::engine::Step, __VA_ARGS__)
#define WHEN(...) WHEN_F(cucumber_cpp::library::engine::Step, __VA_ARGS__)
#define THEN(...) THEN_F(cucumber_cpp::library::engine::Step, __VA_ARGS__)
#define STEP(...) STEP_F(cucumber_cpp::library::engine::Step, __VA_ARGS__)

#endif
