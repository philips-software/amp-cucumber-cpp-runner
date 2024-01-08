#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/JsonTagToSet.hpp"
#include "cucumber-cpp/ResultStates.hpp"
#include "cucumber-cpp/ScenarioRunner.hpp"
#include "cucumber-cpp/TagExpression.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "nlohmann/json.hpp"
#include <iomanip>
#include <ranges>

namespace cucumber_cpp
{
    FeatureRunner::FeatureRunner(Context& programContext, const std::string& tagExpr)
        : tagExpr{ tagExpr }
        , featureContext{ &programContext }
    {}

    void FeatureRunner::Run(nlohmann::json& json)
    {
        BeforeAfterFeatureHookScope featureHookScope{ featureContext, JsonTagsToSet(json["ast"]["feature"]["tags"]) };

        const auto isTagExprSelected = [this](const auto& json)
        {
            return IsTagExprSelected(tagExpr, json["tags"]);
        };

        double totalTime = 0.0;
        std::ranges::for_each(json["scenarios"] | std::views::filter(isTagExprSelected), [this, &json, &totalTime](nlohmann::json& scenarioJson)
            {
                ScenarioRunner scenarioRunner{ featureContext };
                scenarioRunner.Run(scenarioJson);

                totalTime += scenarioJson.value("elapsed", 0.0);
            });

        json["elapsed"] = totalTime;

        const auto containsResult = [](const auto& json)
        {
            return json.contains("result");
        };

        auto onlyScenariosWithResult = json["scenarios"] | std::views::filter(containsResult);

        if (onlyScenariosWithResult.empty())
        {
            // don't set result
        }
        else if (std::ranges::all_of(onlyScenariosWithResult, [](const nlohmann::json& scenarioJson)
                     {
                         return scenarioJson["result"] == result::success;
                     }))
        {
            json["result"] = result::success;
        }
        else
        {
            json["result"] = result::failed;
        }
    }
}
