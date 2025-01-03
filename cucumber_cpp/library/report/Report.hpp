#ifndef REPORT_REPORT_HPP
#define REPORT_REPORT_HPP

#include "cucumber_cpp/library/TraceTime.hpp"
#include "cucumber_cpp/library/engine/ContextManager.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/Result.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace cucumber_cpp
{
    struct FeatureSource;
    struct ScenarioSource;
    struct StepSource;
}

namespace cucumber_cpp::library::report
{
    struct ReportHandlerV2
    {
        virtual ~ReportHandlerV2() = default;

        virtual void FeatureStart(const engine::FeatureInfo& featureInfo) = 0;
        virtual void FeatureEnd(engine::Result result, const engine::FeatureInfo& featureInfo, TraceTime::Duration duration) = 0;

        virtual void RuleStart(const engine::RuleInfo& ruleInfo) = 0;
        virtual void RuleEnd(engine::Result result, const engine::RuleInfo& ruleInfo, TraceTime::Duration duration) = 0;

        virtual void ScenarioStart(const engine::ScenarioInfo& scenarioInfo) = 0;
        virtual void ScenarioEnd(engine::Result result, const engine::ScenarioInfo& scenarioInfo, TraceTime::Duration duration) = 0;

        virtual void StepSkipped(const engine::StepInfo& stepInfo) = 0;
        virtual void StepStart(const engine::StepInfo& stepInfo) = 0;
        virtual void StepEnd(engine::Result result, const engine::StepInfo& stepInfo, TraceTime::Duration duration) = 0;

        virtual void Failure(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) = 0;
        virtual void Error(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) = 0;

        virtual void Trace(const std::string& trace) = 0;

        virtual void Summary(TraceTime::Duration duration) = 0;
    };

    struct Reporters
    {
        void Add(const std::string& name, std::unique_ptr<ReportHandlerV2> reporter);
        void Use(const std::string& name);

        [[nodiscard]] std::vector<std::string> AvailableReporters() const;

    protected:
        void Add(std::unique_ptr<ReportHandlerV2> report);
        std::vector<std::unique_ptr<ReportHandlerV2>>& Storage();

    private:
        std::map<std::string, std::unique_ptr<ReportHandlerV2>, std::less<>> availableReporters;
        std::vector<std::unique_ptr<ReportHandlerV2>> reporters;
    };

    struct ReportForwarder
    {
        struct ProgramScope;
        struct FeatureScope;
        struct RuleScope;
        struct ScenarioScope;
        struct StepScope;

        [[nodiscard]] virtual ProgramScope ProgramStart() = 0;
        [[nodiscard]] virtual FeatureScope FeatureStart() = 0;
        [[nodiscard]] virtual RuleScope RuleStart() = 0;
        [[nodiscard]] virtual ScenarioScope ScenarioStart() = 0;
        [[nodiscard]] virtual StepScope StepStart() = 0;

        virtual void StepSkipped() = 0;

        virtual void Failure(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) = 0;
        virtual void Error(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) = 0;

        virtual void Trace(const std::string& trace) = 0;

        virtual void Summary(TraceTime::Duration duration) = 0;
    };

    struct ReportForwarder::ProgramScope
    {
        ProgramScope(cucumber_cpp::library::engine::ProgramContext& programContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters);
        ~ProgramScope();

    private:
        cucumber_cpp::library::engine::ProgramContext& programContext;
        std::vector<std::unique_ptr<ReportHandlerV2>>& reporters;
    };

    struct ReportForwarder::FeatureScope
    {
        FeatureScope(cucumber_cpp::library::engine::FeatureContext& featureContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters);
        ~FeatureScope();

    private:
        cucumber_cpp::library::engine::FeatureContext& featureContext;
        std::vector<std::unique_ptr<ReportHandlerV2>>& reporters;
    };

    struct ReportForwarder::RuleScope
    {
        RuleScope(cucumber_cpp::library::engine::RuleContext& ruleContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters);
        ~RuleScope();

    private:
        cucumber_cpp::library::engine::RuleContext& ruleContext;
        std::vector<std::unique_ptr<ReportHandlerV2>>& reporters;
    };

    struct ReportForwarder::ScenarioScope
    {
        ScenarioScope(cucumber_cpp::library::engine::ScenarioContext& scenarioContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters);
        ~ScenarioScope();

    private:
        cucumber_cpp::library::engine::ScenarioContext& scenarioContext;
        std::vector<std::unique_ptr<ReportHandlerV2>>& reporters;
    };

    struct ReportForwarder::StepScope
    {
        StepScope(cucumber_cpp::library::engine::StepContext& stepContext, std::vector<std::unique_ptr<ReportHandlerV2>>& reporters);
        ~StepScope();

    private:
        cucumber_cpp::library::engine::StepContext& stepContext;
        std::vector<std::unique_ptr<ReportHandlerV2>>& reporters;
    };

    struct ReportForwarderImpl
        : Reporters
        , ReportForwarder
    {
        explicit ReportForwarderImpl(cucumber_cpp::library::engine::ContextManager& contextManager);

        [[nodiscard]] ProgramScope ProgramStart() override;
        [[nodiscard]] FeatureScope FeatureStart() override;
        [[nodiscard]] RuleScope RuleStart() override;
        [[nodiscard]] ScenarioScope ScenarioStart() override;
        [[nodiscard]] StepScope StepStart() override;

        void StepSkipped() override;

        void Failure(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) override;
        void Error(const std::string& error, std::optional<std::filesystem::path> path = {}, std::optional<std::size_t> line = {}, std::optional<std::size_t> column = {}) override;

        void Trace(const std::string& trace) override;

        void Summary(TraceTime::Duration duration) override;

    private:
        cucumber_cpp::library::engine::ContextManager& contextManager;
    };
}

#endif
