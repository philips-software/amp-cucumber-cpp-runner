#include "cucumber_cpp/library/engine/HookExecutor.hpp"
#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <functional>
#include <set>
#include <string>

namespace cucumber_cpp::library::engine
{

    HookExecutorImpl::HookExecutorImpl(::cucumber_cpp::engine::ContextManager& contextManager)
        : contextManager{ contextManager }
    {}

    void HookExecutorImpl::BeforeAll()
    {
        ExecuteHook(contextManager.ProgramContext(), HookType::beforeAll, {});
    }

    void HookExecutorImpl::AfterAll()
    {
        ExecuteHook(contextManager.ProgramContext(), HookType::afterAll, {});
    }

    void HookExecutorImpl::BeforeFeature()
    {
        ExecuteHook(contextManager.FeatureContext(), HookType::beforeFeature, contextManager.FeatureContext().info.Tags());
    }

    void HookExecutorImpl::AfterFeature()
    {
        ExecuteHook(contextManager.FeatureContext(), HookType::afterFeature, contextManager.FeatureContext().info.Tags());
    }

    void HookExecutorImpl::BeforeScenario()
    {
        ExecuteHook(contextManager.ScenarioContext(), HookType::before, contextManager.ScenarioContext().info.Tags());
    }

    void HookExecutorImpl::AfterScenario()
    {
        ExecuteHook(contextManager.ScenarioContext(), HookType::after, contextManager.ScenarioContext().info.Tags());
    }

    void HookExecutorImpl::BeforeStep()
    {
        ExecuteHook(contextManager.StepContext(), HookType::beforeStep, contextManager.ScenarioContext().info.Tags());
    }

    void HookExecutorImpl::AfterStep()
    {
        ExecuteHook(contextManager.StepContext(), HookType::afterStep, contextManager.ScenarioContext().info.Tags());
    }

    void HookExecutorImpl::ExecuteHook(Context& context, HookType hook, const std::set<std::string, std::less<>>& tags)
    {
        for (const auto& match : HookRegistry::Instance().Query(hook, tags))
            match.factory(context)->Execute();
    }
}
