#ifndef ENGINE_AMBIGUOUSSTEPINFO_HPP
#define ENGINE_AMBIGUOUSSTEPINFO_HPP

#include "cucumber-cpp/StepRegistry.hpp"
#include "cucumber-cpp/engine/StepInfoBase.hpp"
#include <cstddef>
#include <memory>
#include <string>

namespace cucumber_cpp::engine
{
    struct ScenarioInfo;

    struct AmbiguousStepInfo : StepInfoBase
    {
        AmbiguousStepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, std::vector<StepMatch> stepMatch);

        [[nodiscard]] const std::vector<StepMatch>& StepMatches() const;

    private:
        std::vector<struct StepMatch> stepMatches;
    };
}

#endif
