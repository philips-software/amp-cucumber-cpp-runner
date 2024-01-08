#ifndef CUCUMBER_CPP_STEPRUNNER_HPP
#define CUCUMBER_CPP_STEPRUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/Hooks.hpp"

namespace cucumber_cpp
{
    struct StepRunner
    {
        StepRunner(Hooks& hooks, Context& context);

        void Run(nlohmann::json& json, nlohmann::json& scenarioTags);

    private:
        Hooks& hooks;
        Context& context;
    };
}

#endif
