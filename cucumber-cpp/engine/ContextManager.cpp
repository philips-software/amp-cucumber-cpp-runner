#include "cucumber-cpp/engine/ContextManager.hpp"
#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/TraceTime.hpp"
#include "cucumber-cpp/engine/Result.hpp"
#include <memory>
#include <utility>

namespace cucumber_cpp::engine
{
    CurrentContext::CurrentContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : Context{ std::move(contextStorageFactory) }
    {
        Start();
    }

    CurrentContext::CurrentContext(CurrentContext* parent)
        : Context{ parent }
        , parent{ parent }

    {
        Start();
    }

    [[nodiscard]] Result CurrentContext::ExecutionStatus() const
    {
        return executionStatus;
    }

    void CurrentContext::Start()
    {
        traceTime.Start();
    }

    TraceTime::Duration CurrentContext::Duration() const
    {
        return traceTime.Delta();
    }

    void CurrentContext::ExecutionStatus(Result result)
    {
        if (result > executionStatus)
            executionStatus = result;

        if (parent != nullptr)
            parent->ExecutionStatus(result);
    }

    ProgramContext::ProgramContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
        : RunnerContext{ std::move(contextStorageFactory) }
    {
    }

    ContextManager::ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
    {
        activeContextStack.push(std::make_unique<struct ProgramContext>(std::move(contextStorageFactory)));
    }

    RunnerContext& ContextManager::CurrentContext()
    {
        return *activeContextStack.top();
    }

    const RunnerContext& ContextManager::CurrentContext() const
    {
        return *activeContextStack.top();
    }

    RunnerContext& ContextManager::StepContext()
    {
        return *stepContext.top();
    }

    const RunnerContext& ContextManager::StepContext() const
    {
        return *stepContext.top();
    }
}
