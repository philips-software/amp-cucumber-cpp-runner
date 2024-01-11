#include "cucumber-cpp/StepRunner.hpp"
#include "cucumber-cpp/HookScopes.hpp"
#include "cucumber-cpp/JsonTagToSet.hpp"
#include "cucumber-cpp/OnTestPartResultEventListener.hpp"
#include "cucumber-cpp/ResultStates.hpp"
#include "cucumber-cpp/Rtrim.hpp"
#include "cucumber-cpp/StepRegistry.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "nlohmann/json_fwd.hpp"
#include "gtest/gtest.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace cucumber_cpp
{
    namespace
    {
        struct AppendFailureOnTestPartResultEvent : OnTestPartResultEventListener
        {
            explicit AppendFailureOnTestPartResultEvent(nlohmann::json& json)
                : json{ json }
            {
            }

            void OnTestPartResult(const testing::TestPartResult& testPartResult) override
            {
                json["errors"].push_back({ { "file", testPartResult.file_name() },
                    { "line", testPartResult.line_number() },
                    { "message", testPartResult.message() } });
            }

        private:
            nlohmann::json& json;
        };

        const std::map<std::string_view, StepType> stepTypeLut{
            { "Context", StepType::given },
            { "Action", StepType::when },
            { "Outcome", StepType::then },
        };
    }

    StepRunner::StepRunner(Context& context)
        : context{ context }
    {
    }

    void StepRunner::Run(nlohmann::json& json, nlohmann::json& scenarioTags)
    {
        testing::internal::CaptureStdout();
        testing::internal::CaptureStderr();

        try
        {
            AppendFailureOnTestPartResultEvent appendFailureOnTestPartResultEvent{ json };

            const auto stepTypeStr = json["type"].get<std::string>();
            const auto stepMatches = StepRegistry::Instance().Query(stepTypeLut.at(stepTypeStr), json["text"]);

            if (const auto& step = stepMatches.front(); stepMatches.size() == 1)
            {
                TraceTime traceTime{ json };

                BeforeAfterStepHookScope stepHookScope{ context, JsonTagsToSet(scenarioTags) };

                step.factory(context, json["argument"]["dataTable"])->Execute(step.regexMatch->Matches());
            }

            if (json.count("errors") == 0)
            {
                json["result"] = result::success;
            }
            else
            {
                json["result"] = result::failed;
            }
        }
        catch ([[maybe_unused]] const StepRegistry::StepNotFound& e)
        {
            json["result"] = result::undefined;
        }
        catch (const std::exception& e)
        {
            json["error"].push_back({ { "file", "unknown" },
                { "line", 0 },
                { "message", std::string{ "Exception thrown: " } + e.what() } });
            json["result"] = result::error;
        }
        catch (...)
        {
            json["error"].push_back({ { "file", "unknown" },
                { "line", 0 },
                { "message", "Unknown exception thrown" } });
            json["result"] = result::error;
        }

        if (auto str = testing::internal::GetCapturedStdout(); !str.empty())
        {
            Rtrim(str);
            json["stdout"] = str;
        }

        if (auto str = testing::internal::GetCapturedStderr(); !str.empty())
        {
            Rtrim(str);
            json["stderr"] = str;
        }
    }
}
