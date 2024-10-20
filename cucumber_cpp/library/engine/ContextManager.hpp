#ifndef ENGINE_CONTEXTMANAGER_HPP
#define ENGINE_CONTEXTMANAGER_HPP

#include "cucumber_cpp/library/Context.hpp"
#include "cucumber_cpp/library/TraceTime.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

namespace cucumber_cpp::engine
{
    struct CurrentContext : Context
    {
        explicit CurrentContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory);
        explicit CurrentContext(CurrentContext* parent);

        [[nodiscard]] Result ExecutionStatus() const;

        void Start();
        [[nodiscard]] TraceTime::Duration Duration() const;

    protected:
        void ExecutionStatus(Result result);

    private:
        CurrentContext* parent{ nullptr };

        Result executionStatus{ Result::passed };
        TraceTime traceTime;
    };

    struct RunnerContext : CurrentContext
    {
        using CurrentContext::CurrentContext;
        using CurrentContext::ExecutionStatus;

        void AppendFailure(testing::TestPartResult::Type type, std::filesystem::path srcfile, int line_num, std::string message);

    private:
        struct Failure
        {
            testing::TestPartResult::Type type;
            std::filesystem::path srcfile;
            int line_num;
            std::string message;
        };

        std::vector<std::shared_ptr<Failure>> failures;
    };

    struct ProgramContext : RunnerContext
    {
        explicit ProgramContext(std::shared_ptr<ContextStorageFactory> contextStorageFactory);
    };

    template<class T>
    struct NestedContext : RunnerContext
    {
        NestedContext(RunnerContext& parent, const T& info)
            : RunnerContext{ &parent }
            , info{ info }
        {}

        const T& info;
    };

    using FeatureContext = NestedContext<FeatureInfo>;
    using RuleContext = NestedContext<RuleInfo>;
    using ScenarioContext = NestedContext<ScenarioInfo>;
    using StepContext = NestedContext<StepInfo>;
    using NestedStepContext = NestedContext<NestedStepInfo>;

    struct ContextNotAvailable : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    struct ContextManager
    {
        explicit ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory);

        cucumber_cpp::engine::ProgramContext& ProgramContext();
        cucumber_cpp::engine::ProgramContext& ProgramContext() const;

        void CreateFeatureContext(const FeatureInfo& featureInfo);
        void DisposeFeatureContext();
        cucumber_cpp::engine::FeatureContext& FeatureContext();

        void CreateRuleContext(const RuleInfo& ruleInfo);
        void DisposeRuleContext();
        cucumber_cpp::engine::RuleContext& RuleContext();

        void CreateScenarioContext(const ScenarioInfo& scenarioInfo);
        void DisposeScenarioContext();
        cucumber_cpp::engine::ScenarioContext& ScenarioContext();

        void CreateStepContext(const StepInfo& stepInfo);
        void DisposeStepContext();
        cucumber_cpp::engine::StepContext& StepContext();

        cucumber_cpp::engine::RunnerContext& CurrentContext();

    private:
        std::shared_ptr<cucumber_cpp::engine::ProgramContext> programContext;
        std::shared_ptr<cucumber_cpp::engine::FeatureContext> featureContext;
        std::shared_ptr<cucumber_cpp::engine::RuleContext> ruleContext;
        std::shared_ptr<cucumber_cpp::engine::ScenarioContext> scenarioContext;

        std::stack<std::shared_ptr<cucumber_cpp::engine::RunnerContext>> runnerContext;
        std::stack<std::shared_ptr<cucumber_cpp::engine::StepContext>> stepContext;
    };
}

#endif
