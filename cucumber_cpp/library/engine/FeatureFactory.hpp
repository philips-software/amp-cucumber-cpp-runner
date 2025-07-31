#ifndef ENGINE_FEATUREFACTORY_HPP
#define ENGINE_FEATUREFACTORY_HPP

#include "cucumber/gherkin/app.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include "cucumber_cpp/library/report/Report.hpp"
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct FeatureTreeFactory
    {
        FeatureTreeFactory(StepRegistry& stepRegistry, report::ReportForwarder& reportHandler);

        [[nodiscard]] std::unique_ptr<StepInfo> CreateStepInfo(StepType stepType, std::string stepText, const ScenarioInfo& scenarioInfo, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table) const;

        [[nodiscard]] std::unique_ptr<FeatureInfo> Create(const std::filesystem::path& path, std::string_view tagExpression) const;

    private:
        StepRegistry& stepRegistry;
        report::ReportForwarder& reportHandler;
    };
}

#endif
