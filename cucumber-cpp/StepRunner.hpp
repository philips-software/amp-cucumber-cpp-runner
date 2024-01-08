#ifndef CUCUMBER_CPP_STEPRUNNER_HPP
#define CUCUMBER_CPP_STEPRUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/Hooks.hpp"

namespace cucumber_cpp
{
    struct StepRunner
    {
        StepRunner(Context& context);

        void Run(nlohmann::json& json, nlohmann::json& scenarioTags);

    private:
        Context& context;
    };
}

#endif
