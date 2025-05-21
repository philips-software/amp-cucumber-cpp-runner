#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <functional>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace cucumber_cpp::library::engine
{
    namespace
    {
        constexpr HookPair programHooks{ HookType::beforeAll, HookType::afterAll };
        constexpr HookPair featureHooks{ HookType::beforeFeature, HookType::afterFeature };
        constexpr HookPair scenarioHooks{ HookType::before, HookType::after };
        constexpr HookPair stepHooks{ HookType::beforeStep, HookType::afterStep };

        void ExecuteHook(cucumber_cpp::library::engine::RunnerContext& runnerContext, HookType hook, const std::set<std::string, std::less<>>& tags)
        {
            if (runnerContext.InheritedExecutionStatus() == Result::passed)
                for (const auto& match : HookRegistry::Instance().Query(hook, tags))
                {
                    match.factory(runnerContext)->Execute();

                    if (runnerContext.ExecutionStatus() != cucumber_cpp::library::engine::Result::passed)
                        return;
                }
        }
    }

    HookExecutor::ScopedHook::ScopedHook(cucumber_cpp::library::engine::RunnerContext& runnerContext, HookPair hookPair, const std::set<std::string, std::less<>>& tags)
        : runnerContext{ runnerContext }
        , hookPair{ hookPair }
        , tags{ tags }
    {
        ExecuteHook(runnerContext, hookPair.before, tags);
    }

    HookExecutor::ScopedHook::~ScopedHook()
    {
        ExecuteHook(runnerContext, hookPair.after, tags);
    }

    HookExecutor::ProgramScope::ProgramScope(cucumber_cpp::library::engine::ContextManager& contextManager)
        : ScopedHook{ contextManager.ProgramContext(), programHooks, {} }
    {
    }

    HookExecutor::FeatureScope::FeatureScope(cucumber_cpp::library::engine::ContextManager& contextManager)
        : ScopedHook{ contextManager.FeatureContext(), featureHooks, contextManager.FeatureContext().info.Tags() }
    {}

    HookExecutor::ScenarioScope::ScenarioScope(cucumber_cpp::library::engine::ContextManager& contextManager)
        : ScopedHook{ contextManager.ScenarioContext(), scenarioHooks, contextManager.ScenarioContext().info.Tags() }
    {}

    HookExecutor::StepScope::StepScope(cucumber_cpp::library::engine::ContextManager& contextManager)
        : ScopedHook{ contextManager.ScenarioContext(), stepHooks, contextManager.ScenarioContext().info.Tags() }
    {}

    HookExecutorImpl::HookExecutorImpl(cucumber_cpp::library::engine::ContextManager& contextManager)
        : contextManager{ contextManager }
    {}

    HookExecutor::ProgramScope HookExecutorImpl::BeforeAll()
    {
        return ProgramScope{ contextManager };
    }

    std::optional<HookExecutor::FeatureScope> HookExecutorImpl::FeatureStart(cucumber_cpp::library::engine::ContextManager& contextManager)
    {
        if (contextManager.FeatureContext().InheritedExecutionStatus() != Result::passed)
            return std::nullopt;
        return std::make_optional<FeatureScope>(contextManager);
    }

    std::optional<HookExecutor::ScenarioScope> HookExecutorImpl::ScenarioStart(cucumber_cpp::library::engine::ContextManager& contextManager)
    {
        if (contextManager.ScenarioContext().InheritedExecutionStatus() != Result::passed)
            return std::nullopt;
        return std::make_optional<ScenarioScope>(contextManager);
    }

    HookExecutor::StepScope HookExecutorImpl::StepStart()
    {
        return StepScope{ contextManager };
    }
}
