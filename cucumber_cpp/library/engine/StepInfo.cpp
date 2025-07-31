#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <cstddef>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::library::engine
{
    StepInfo::StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, StepType type, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table)
        : scenarioInfo{ scenarioInfo }
        , text{ std::move(text) }
        , type{ type }
        , line{ line }
        , column{ column }
        , table{ std::move(table) }
    {
    }

    StepInfo::StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, StepType type, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, StepRegistry::StepMatch stepMatch)
        : scenarioInfo{ scenarioInfo }
        , text{ std::move(text) }
        , type{ type }
        , line{ line }
        , column{ column }
        , table{ std::move(table) }
        , stepMatch{ std::move(stepMatch) }
    {
    }

    StepInfo::StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, StepType type, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, std::vector<StepRegistry::StepMatch> stepMatches)
        : scenarioInfo{ scenarioInfo }
        , text{ std::move(text) }
        , type{ type }
        , line{ line }
        , column{ column }
        , table{ std::move(table) }
        , stepMatch{ std::move(stepMatches) }
    {
    }

    const struct ScenarioInfo& StepInfo::ScenarioInfo() const
    {
        return scenarioInfo;
    }

    const std::string& StepInfo::Text() const
    {
        return text;
    }

    StepType StepInfo::Type() const
    {
        return type;
    }

    std::size_t StepInfo::Line() const
    {
        return line;
    }

    std::size_t StepInfo::Column() const
    {
        return column;
    }

    [[nodiscard]] const std::vector<std::vector<TableValue>>& StepInfo::Table() const
    {
        return table;
    }

    const std::variant<std::monostate, StepRegistry::StepMatch, std::vector<StepRegistry::StepMatch>>& StepInfo::StepMatch() const
    {
        return stepMatch;
    }
}
