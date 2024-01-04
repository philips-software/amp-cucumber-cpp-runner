#include "cucumber-cpp/CucumberRunner.hpp"
#include "cucumber-cpp/FeatureRunner.hpp"
#include "cucumber-cpp/ResultStates.hpp"
#include "cucumber-cpp/ScenarioRunner.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "nlohmann/json_fwd.hpp"

namespace cucumber_cpp
{
    CucumberRunner::CucumberRunner(const std::vector<std::string_view>& args, Hooks& hooks, StepRepository& stepRepository, const std::string& tagExpr, std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : hooks{ hooks }
        , stepRepository{ stepRepository }
        , tagExpr{ tagExpr }
        , programContext(contextStorageFactory)
    {
        programContext.InsertAt("args", args);
    }

    void CucumberRunner::Run(nlohmann::json& json)
    {
        BeforeAfterAllScope programHookeScope{ hooks, programContext };

        double totalTime = 0.0;

        std::ranges::for_each(json["features"], [this, &json, &totalTime](nlohmann::json& featureJson)
            {
                FeatureRunner featureRunner{ hooks, stepRepository, programContext, tagExpr };
                featureRunner.Run(featureJson);

                totalTime += featureJson.value("elapsed", 0.0);
            });

        json["elapsed"] = totalTime;

        auto onlyFeaturesWithResult = json["features"] | std::views::filter([](const auto& json)
                                                             {
                                                                 return json.contains("result");
                                                             });

        if (onlyFeaturesWithResult.empty())
        {
            // don't set result
        }
        else if (std::ranges::all_of(onlyFeaturesWithResult, [](const auto& featureJson)
                     {
                         return featureJson["result"] == result::success;
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
