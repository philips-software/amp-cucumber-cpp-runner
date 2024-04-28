#ifndef CUCUMBER_CPP_CONTEXTMANAGER_HPP
#define CUCUMBER_CPP_CONTEXTMANAGER_HPP

#include "cucumber-cpp/Context.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include "cucumber-cpp/report/Report.hpp"
#include <memory>

namespace cucumber_cpp
{
    struct ProgramContext : Context
    {
        [[nodiscard]] report::ReportHandler::Result ExecutionStatus() const
        {
            return executionStatus;
        }

    private:
        engine::FeatureInfo featureInfo;
        report::ReportHandler::Result executionStatus{ report::ReportHandler::Result::success };
    };

    struct FeatureContext : Context
    {
        [[nodiscard]] report::ReportHandler::Result ExecutionStatus() const
        {
            return executionStatus;
        }

    private:
        engine::ScenarioInfo scenarioInfo;
        report::ReportHandler::Result executionStatus{ report::ReportHandler::Result::success };
    };

    struct ScenarioContext : Context
    {
        [[nodiscard]] report::ReportHandler::Result ExecutionStatus() const
        {
            return executionStatus;
        }

    private:
        engine::StepInfo stepInfo;
        report::ReportHandler::Result executionStatus{ report::ReportHandler::Result::success };
    };

    struct ContextManager
    {
        explicit ContextManager(std::shared_ptr<ContextStorageFactory> contextStorageFactory)
            : contextStorageFactory{ std::move(contextStorageFactory) }
        {}

        std::shared_ptr<ProgramContext> CreateProgramContext()
        {
            return programContext = std::make_shared<ProgramContext>(*contextStorageFactory);
        }

        std::shared_ptr<FeatureContext> CreateFeatureContext()
        {
            return featureContext = std::make_shared<FeatureContext>(*programContext);
        }

        std::shared_ptr<ScenarioContext> CreateScenarioContext()
        {
            return scenarioContext = std::make_shared<ScenarioContext>(*scenarioContext);
        }

    private:
        std::shared_ptr<ContextStorageFactory> contextStorageFactory;
        std::shared_ptr<ProgramContext> programContext;
        std::shared_ptr<FeatureContext> featureContext;
        std::shared_ptr<ScenarioContext> scenarioContext;
    };
}

#endif
