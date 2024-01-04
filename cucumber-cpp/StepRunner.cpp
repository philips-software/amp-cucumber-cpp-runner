#include "cucumber-cpp/StepRunner.hpp"
#include "cucumber-cpp/OnTestPartResultEventListener.hpp"
#include "cucumber-cpp/ResultStates.hpp"
#include "cucumber-cpp/Rtrim.hpp"
#include "cucumber-cpp/Steps.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "nlohmann/json_fwd.hpp"
#include "gtest/gtest.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
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

    StepRunner::StepRunner(Hooks& hooks, StepRepository& stepRepository, Context& context)
        : hooks{ hooks }
        , stepRepository{ stepRepository }
        , context{ context }
    {
    }

    void StepRunner::Run(nlohmann::json& json, nlohmann::json& scenarioTags)
    {
        BeforeAfterStepHookScope stepHookScope{ hooks, context, scenarioTags };
        testing::internal::CaptureStdout();
        testing::internal::CaptureStderr();

        try
        {
            AppendFailureOnTestPartResultEvent appendFailureOnTestPartResultEvent{ json };

            const auto stepTypeStr = json["type"].get<std::string>();
            const auto stepMatch = stepRepository.Get(stepTypeLut.at(stepTypeStr), json["text"]);

            {
                TraceTime traceTime{ json };
                stepMatch.step->Run(context, stepMatch.regexMatch->Matches(), json["argument"]["dataTable"]);
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
        catch ([[maybe_unused]] const StepNotFoundException& e)
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
