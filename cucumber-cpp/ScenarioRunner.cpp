#include "cucumber-cpp/ScenarioRunner.hpp"
#include "cucumber-cpp/ResultStates.hpp"
#include "cucumber-cpp/StepRunner.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "nlohmann/json.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <memory>

namespace cucumber_cpp
{
    namespace
    {
        struct StepRunnerStrategy : RunStepStrategy
        {
            void Run(StepRunner& stepRunner, nlohmann::json& json, nlohmann::json& scenarioTags) override
            {
                stepRunner.Run(json, scenarioTags);
            }
        };

        struct SkipStepStrategy : RunStepStrategy
        {
            void Run([[maybe_unused]] StepRunner& stepRunner, nlohmann::json& json, nlohmann::json& scenarioTags) override
            {
                json["result"] = result::skipped;
            }
        };
    }

    ScenarioRunner::ScenarioRunner(Hooks& hooks, Context& programContext)
        : hooks{ hooks }
        , scenarioContext{ &programContext }
        , runStepStrategy{ std::make_unique<StepRunnerStrategy>() }
    {
    }

    void ScenarioRunner::Run(nlohmann::json& scenarioJson)
    {

        BeforeAfterHookScope scenarioHookScope{ hooks, scenarioContext, scenarioJson["tags"] };
        double totalTime = 0.0;

        std::ranges::for_each(scenarioJson["steps"], [&scenarioJson, this, &totalTime](nlohmann::json& stepJson)
            {
                StepRunner stepRunner{ hooks, scenarioContext };
                runStepStrategy->Run(stepRunner, stepJson, scenarioJson["tags"]);

                if (auto result = stepJson["result"]; result != result::success && result != result::skipped)
                {
                    runStepStrategy = std::make_unique<SkipStepStrategy>();
                }

                totalTime += stepJson.value("elapsed", 0.0);
            });

        scenarioJson["elapsed"] = totalTime;

        auto iter = std::ranges::find_if_not(scenarioJson["steps"], [](auto stepJson)
            {
                return stepJson["result"] == result::success;
            });

        if (iter == scenarioJson["steps"].end())
        {
            scenarioJson["result"] = result::success;
        }
        else
        {
            scenarioJson["result"] = (*iter)["result"];
        }
    }

    void ScenarioRunner::OnTestPartResult(const testing::TestPartResult& testPartResult)
    {
        runStepStrategy = std::make_unique<SkipStepStrategy>();
    }
}
