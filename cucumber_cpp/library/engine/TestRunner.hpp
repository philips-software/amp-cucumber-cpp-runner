#ifndef ENGINE_TESTRUNNER_HPP
#define ENGINE_TESTRUNNER_HPP

#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/TestExecution.hpp"
#include <memory>
#include <string>
#include <vector>

namespace cucumber_cpp::library::report
{
    struct ReportHandlerV2;
}

namespace cucumber_cpp::library::engine
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
        virtual void NestedStep(StepType type, std::string step) = 0;

    private:
        static TestRunner* instance; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    };

    struct TestRunnerImpl : TestRunner
    {
        explicit TestRunnerImpl(cucumber_cpp::library::engine::TestExecution& testExecution);
        virtual ~TestRunnerImpl() = default;

        void Run(const std::vector<std::unique_ptr<FeatureInfo>>& feature) override;

        void NestedStep(StepType type, std::string step) override;

    private:
        void ExecuteSteps(const ScenarioInfo& scenario);
        void RunScenario(const ScenarioInfo& scenario);
        void RunScenarios(const std::vector<std::unique_ptr<ScenarioInfo>>& scenarios);
        void RunRule(const RuleInfo& rule);
        void RunRules(const std::vector<std::unique_ptr<RuleInfo>>& rules);
        void RunFeature(const FeatureInfo& feature);

        cucumber_cpp::library::engine::TestExecution& testExecution;

        const ScenarioInfo* currentScenario = nullptr;
    };
}

#endif
