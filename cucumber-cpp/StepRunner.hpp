#ifndef CUCUMBER_CPP_STEPRUNNER_HPP
#define CUCUMBER_CPP_STEPRUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/Hooks.hpp"
#include "cucumber-cpp/Steps.hpp"

namespace cucumber_cpp
{
    struct StepRunner
    {
        StepRunner(Hooks& hooks, StepRepository& stepRepository, Context& context);

        void Run(nlohmann::json& json, nlohmann::json& scenarioTags);

    private:
        Hooks& hooks;
        StepRepository& stepRepository;
        Context& context;
    };
}

#endif
