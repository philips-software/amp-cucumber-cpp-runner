#include "cucumber-cpp/engine/StepInfo.hpp"
#include "cucumber-cpp/StepRegistry.hpp"

namespace cucumber_cpp::engine
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

    StepInfo::StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, StepType type, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, struct StepMatch stepMatch)
        : scenarioInfo{ scenarioInfo }
        , text{ std::move(text) }
        , type{ type }
        , line{ line }
        , column{ column }
        , table{ std::move(table) }
        , stepMatch{ std::move(stepMatch) }
    {
    }

    StepInfo::StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, StepType type, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, std::vector<struct StepMatch> stepMatches)
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

    const std::variant<std::monostate, struct StepMatch, std::vector<struct StepMatch>>& StepInfo::StepMatch() const
    {
        return stepMatch;
    }
}
