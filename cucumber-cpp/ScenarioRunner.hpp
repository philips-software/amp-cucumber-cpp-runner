#ifndef CUCUMBER_CPP_SCENARIORUNNER_HPP
#define CUCUMBER_CPP_SCENARIORUNNER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/Hooks.hpp"
#include "cucumber-cpp/OnTestPartResultEventListener.hpp"
#include "cucumber-cpp/StepRunner.hpp"
#include "cucumber-cpp/Steps.hpp"

namespace cucumber_cpp
{

    struct RunStepStrategy
    {
        virtual ~RunStepStrategy() = default;
        virtual void Run(StepRunner& stepRunner, nlohmann::json& json, nlohmann::json& scenarioTags) = 0;
    };

    struct ScenarioRunner : OnTestPartResultEventListener
    {
        ScenarioRunner(Hooks& hooks, StepRepository& stepRepository, Context& programContext);

        void Run(nlohmann::json& json);

        // implementation of OnTestPartResultEventListener
        void OnTestPartResult(const testing::TestPartResult& testPartResult) override;

    private:
        Hooks& hooks;
        StepRepository& stepRepository;
        Context scenarioContext;
        std::unique_ptr<RunStepStrategy> runStepStrategy;
    };
}

#endif
