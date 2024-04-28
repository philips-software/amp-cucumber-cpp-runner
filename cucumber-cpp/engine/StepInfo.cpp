#include "cucumber-cpp/engine/StepInfo.hpp"
#include "cucumber-cpp/engine/StepInfoBase.hpp"
#include <optional>
#include <utility>

namespace cucumber_cpp::engine
{
    StepInfo::StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, struct StepMatch stepMatch)
        : StepInfoBase(scenarioInfo, std::move(text), line, column, std::move(table))
        , stepMatch{ std::move(stepMatch) }
    {}

    const struct StepMatch& StepInfo::StepMatch() const
    {
        return stepMatch;
    }

}
