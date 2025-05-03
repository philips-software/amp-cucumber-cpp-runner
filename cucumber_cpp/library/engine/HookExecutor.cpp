#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <functional>
#include <set>
#include <stdexcept>
#include <string>

namespace cucumber_cpp::library::engine
{
    namespace
    {
        constexpr HookPair programHooks{ HookType::beforeAll, HookType::afterAll };
        constexpr HookPair featureHooks{ HookType::beforeFeature, HookType::afterFeature };
        constexpr HookPair scenarioHooks{ HookType::before, HookType::after };
        constexpr HookPair stepHooks{ HookType::beforeStep, HookType::afterStep };

        struct HookFailed : std::runtime_error
        {
            using runtime_error::runtime_error;
        };

        void ExecuteHook(cucumber_cpp::library::engine::RunnerContext& runnerContext, HookType hook, const std::set<std::string, std::less<>>& tags)
        {
            for (const auto& match : HookRegistry::Instance().Query(hook, tags))
            {
                match.factory(runnerContext)->Execute();

                if (runnerContext.ExecutionStatus() != cucumber_cpp::library::engine::Result::passed)
                    throw HookFailed{ "hook failed" };
            }
        }
    }

    HookExecutor::ScopedHook::ScopedHook(cucumber_cpp::library::engine::RunnerContext& runnerContext, HookPair hookPair, const std::set<std::string, std::less<>>& tags)
        : runnerContext{ runnerContext }
        , hookPair{ hookPair }
        , tags{ tags }
    {
        try
        {
            ExecuteHook(runnerContext, hookPair.before, tags);
        }
        catch (HookFailed&)
        {
            /* This throw ensures no tests are executed in and below the current scope. */
            throw HookFailed{ "Error during BEFORE_x_HOOK" };
        }
    }

    HookExecutor::ScopedHook::~ScopedHook()
    {
        try
        {
            ExecuteHook(runnerContext, hookPair.after, tags);
        }
        catch (HookFailed&)
        {
            /*
            Can't throw from a destructor. Error state will be handled by calling functions.

            Errors during AFTER_ALL hook is not a problem, because no other hooks will be executed.
            */
        }
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

    HookExecutor::FeatureScope HookExecutorImpl::FeatureStart()
    {
        return FeatureScope{ contextManager };
    }

    HookExecutor::ScenarioScope HookExecutorImpl::ScenarioStart()
    {
        return ScenarioScope{ contextManager };
    }

    HookExecutor::StepScope HookExecutorImpl::StepStart()
    {
        return StepScope{ contextManager };
    }
}
