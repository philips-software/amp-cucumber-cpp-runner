#include "cucumber-cpp/engine/AmbiguousStepInfo.hpp"
#include <optional>
#include <utility>

namespace cucumber_cpp::engine
{
    AmbiguousStepInfo::AmbiguousStepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, std::vector<StepMatch> stepMatches)
        : StepInfoBase{ scenarioInfo, std::move(text), line, column, std::move(table) }
        , stepMatches{ std::move(stepMatches) }
    {}

    const std::vector<StepMatch>& AmbiguousStepInfo::StepMatches() const
    {
        return stepMatches;
    }
}
