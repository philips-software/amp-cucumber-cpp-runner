#ifndef ENGINE_STEPINFO_HPP
#define ENGINE_STEPINFO_HPP

#include "cucumber_cpp/library/StepRegistry.hpp"
#include "cucumber_cpp/library/engine/StepType.hpp"
#include "cucumber_cpp/library/engine/Table.hpp"
#include <cstddef>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

namespace cucumber_cpp::engine
{
    struct ScenarioInfo;

    struct StepInfo
    {
        StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, StepType type, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table);
        StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, StepType type, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, StepMatch);
        StepInfo(const struct ScenarioInfo& scenarioInfo, std::string text, StepType type, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, std::vector<StepMatch>);

        [[nodiscard]] const struct ScenarioInfo& ScenarioInfo() const;

        [[nodiscard]] const std::string& Text() const;
        [[nodiscard]] StepType Type() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] const std::vector<std::vector<TableValue>>& Table() const;
        [[nodiscard]] const std::variant<std::monostate, struct StepMatch, std::vector<struct StepMatch>>& StepMatch() const;

    private:
        const struct ScenarioInfo& scenarioInfo;

        std::string text;
        StepType type;

        std::size_t line;
        std::size_t column;

        std::vector<std::vector<TableValue>> table;
        std::variant<std::monostate, struct StepMatch, std::vector<struct StepMatch>> stepMatch;
    };

    struct NestedStepInfo
    {
        NestedStepInfo(std::string text, StepType type, std::filesystem::path, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table);
        NestedStepInfo(std::string text, StepType type, std::filesystem::path, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, StepMatch);
        NestedStepInfo(std::string text, StepType type, std::filesystem::path, std::size_t line, std::size_t column, std::vector<std::vector<TableValue>> table, std::vector<StepMatch>);

        [[nodiscard]] const std::filesystem::path& FilePath() const;

        [[nodiscard]] const std::string& Text() const;
        [[nodiscard]] StepType Type() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] const std::vector<std::vector<TableValue>>& Table() const;
        [[nodiscard]] const std::variant<std::monostate, struct StepMatch, std::vector<struct StepMatch>>& StepMatch() const;

    private:
        const std::filesystem::path filePath;

        std::string text;
        StepType type;

        std::size_t line;
        std::size_t column;

        std::vector<std::vector<TableValue>> table;
        std::variant<std::monostate, struct StepMatch, std::vector<struct StepMatch>> stepMatch;
    };
}

#endif
