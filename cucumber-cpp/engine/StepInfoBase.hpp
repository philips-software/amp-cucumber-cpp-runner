#ifndef ENGINE_STEPINFOBASE_HPP
#define ENGINE_STEPINFOBASE_HPP

#include "cucumber-cpp/StepRegistry.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace cucumber_cpp::engine
{
    struct ScenarioInfo;

    struct StepInfoBase
    {
        StepInfoBase(const struct ScenarioInfo& scenarioInfo, std::string text, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table);

        [[nodiscard]] const struct ScenarioInfo& ScenarioInfo() const;

        [[nodiscard]] const std::string& Text() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] const std::vector<std::vector<TableValue>>& Table() const;

    private:
        const struct ScenarioInfo& scenarioInfo;

        std::string text;

        std::size_t line;
        std::size_t column;

        std::vector<std::vector<TableValue>> table;
    };
}

#endif
