#ifndef ENGINE_RULEINFO_HPP
#define ENGINE_RULEINFO_HPP

#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cucumber_cpp::library::engine
{
    struct FeatureInfo;

    struct RuleInfo
    {
        RuleInfo(const FeatureInfo& featureInfo, std::string id, std::string title, std::string description, std::size_t line, std::size_t column);

        [[nodiscard]] const struct FeatureInfo& FeatureInfo() const;

        [[nodiscard]] std::string_view Id() const;

        [[nodiscard]] const std::string& Title() const;
        [[nodiscard]] const std::string& Description() const;

        [[nodiscard]] const std::filesystem::path& Path() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] std::vector<std::unique_ptr<ScenarioInfo>>& Scenarios();
        [[nodiscard]] const std::vector<std::unique_ptr<ScenarioInfo>>& Scenarios() const;

    private:
        const struct FeatureInfo& featureInfo;

        std::string id;

        std::string title;
        std::string description;

        std::size_t line;
        std::size_t column;

        std::vector<std::unique_ptr<ScenarioInfo>> scenarios;
    };
}

#endif
