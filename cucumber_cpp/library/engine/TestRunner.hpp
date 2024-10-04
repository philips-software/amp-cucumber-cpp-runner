#ifndef ENGINE_TESTRUNNER_HPP
#define ENGINE_TESTRUNNER_HPP

#include "cucumber_cpp/library/HookRegistry.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cucumber_cpp::report
{
    struct ReportHandlerV2;
}

namespace cucumber_cpp::engine
{
    struct RunPolicy
    {
        virtual ~RunPolicy() = default;

        virtual void ExecuteStep(ContextManager& contextManager, const StepInfo& stepInfo, const StepMatch& stepMatch) const = 0;
        virtual bool ExecuteHook(ContextManager& context, HookType hook, const std::set<std::string, std::less<>>& tags) const = 0;
    };

    struct RunTestPolicy : RunPolicy
    {
        void ExecuteStep(ContextManager& contextManager, const StepInfo& stepInfo, const StepMatch& stepMatch) const override;
        bool ExecuteHook(ContextManager& context, HookType hook, const std::set<std::string, std::less<>>& tags) const override;
    };

    struct DryRunPolicy : RunPolicy
    {
        void ExecuteStep(ContextManager& contextManager, const StepInfo& stepInfo, const StepMatch& stepMatch) const override;
        bool ExecuteHook(ContextManager& context, HookType hook, const std::set<std::string, std::less<>>& tags) const override;
    };

    static const RunTestPolicy runTest;
    static const DryRunPolicy dryRun;

    namespace TestRunner
    {
        struct PendingException
        {};

        struct StepNotFoundException
        {};

        struct AmbiguousStepException
        {};

        void Run(ContextManager& contextManager, const std::vector<std::unique_ptr<FeatureInfo>>& feature, report::ReportHandlerV2& reportHandler, const RunPolicy& runPolicy);
    }
}

#endif
