#include "cucumber-cpp/engine/StepInfoBase.hpp"

namespace cucumber_cpp::engine
{
    StepInfoBase::StepInfoBase(const struct ScenarioInfo& scenarioInfo, std::string text, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table)
        : scenarioInfo{ scenarioInfo }
        , text{ std::move(text) }
        , line{ line }
        , column{ column }
        , table{ std::move(table) }
    {}

    const struct ScenarioInfo& StepInfoBase::ScenarioInfo() const
    {
        return scenarioInfo;
    }

    const std::string& StepInfoBase::Text() const
    {
        return text;
    }

    std::size_t StepInfoBase::Line() const
    {
        return line;
    }

    std::size_t StepInfoBase::Column() const
    {
        return column;
    }

    [[nodiscard]] const std::vector<std::vector<TableValue>>& StepInfoBase::Table() const
    {
        return table;
    }
}
