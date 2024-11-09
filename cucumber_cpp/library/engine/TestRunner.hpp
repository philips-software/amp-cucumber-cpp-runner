#ifndef ENGINE_TESTRUNNER_HPP
#define ENGINE_TESTRUNNER_HPP

#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include <memory>
#include <vector>

namespace cucumber_cpp::report
{
    struct ReportHandlerV2;
}

namespace cucumber_cpp::engine
{
    struct TestRunner
    {
        struct PendingException
        {};

        struct StepNotFoundException
        {};

        struct AmbiguousStepException
        {};

    protected:
        TestRunner();
        ~TestRunner();

    public:
        TestRunner(const TestRunner&) = delete;

        static TestRunner& Instance();

        virtual void Run(const std::vector<std::unique_ptr<FeatureInfo>>& feature) = 0;

    private:
        static TestRunner* instance;
    };

    struct TestRunnerImpl : TestRunner
    {
        explicit TestRunnerImpl(::cucumber_cpp::library::engine::TestExecution& testExecution);

        void Run(const std::vector<std::unique_ptr<FeatureInfo>>& feature) override;

    private:
        void ExecuteSteps(const ScenarioInfo& scenario);
        void RunScenario(const std::unique_ptr<ScenarioInfo>& scenario);
        void RunScenarios(const std::vector<std::unique_ptr<ScenarioInfo>>& scenarios);
        void RunRule(const std::unique_ptr<RuleInfo>& rule);
        void RunRules(const std::vector<std::unique_ptr<RuleInfo>>& rules);
        void RunFeature(const FeatureInfo& feature);

        ::cucumber_cpp::library::engine::TestExecution& testExecution;
    };
}

#endif
