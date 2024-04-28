#ifndef ENGINE_STEPINFO_HPP
#define ENGINE_STEPINFO_HPP

#include "cucumber-cpp/StepRegistry.hpp"
#include "cucumber-cpp/engine/StepInfoBase.hpp"
#include <cstddef>
#include <memory>
#include <string>

namespace cucumber_cpp::engine
{
    struct StepInfo : StepInfoBase
    {
        StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, StepMatch stepMatch);

        [[nodiscard]] const struct StepMatch& StepMatch() const;

    private:
        struct StepMatch stepMatch;
    };
}

#endif
