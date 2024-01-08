#ifndef CUCUMBER_CPP_SCENARIORUNNER_HPP
#define CUCUMBER_CPP_SCENARIORUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/Hooks.hpp"
#include "cucumber-cpp/OnTestPartResultEventListener.hpp"
#include "cucumber-cpp/StepRunner.hpp"

namespace cucumber_cpp
{

    struct RunStepStrategy
    {
        virtual ~RunStepStrategy() = default;
        virtual void Run(StepRunner& stepRunner, nlohmann::json& json, nlohmann::json& scenarioTags) = 0;
    };

    struct ScenarioRunner : OnTestPartResultEventListener
    {
        ScenarioRunner(Context& programContext);

        void Run(nlohmann::json& json);

        // implementation of OnTestPartResultEventListener
        void OnTestPartResult(const testing::TestPartResult& testPartResult) override;

    private:
        Context scenarioContext;
        std::unique_ptr<RunStepStrategy> runStepStrategy;
    };
}

#endif
