#ifndef ENGINE_SCENARIOINFO_HPP
#define ENGINE_SCENARIOINFO_HPP

#include "cucumber_cpp/engine/RuleInfo.hpp"
#include "cucumber_cpp/engine/StepInfo.hpp"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace cucumber_cpp::engine
{
    struct FeatureInfo;

    struct ScenarioInfo
    {
        ScenarioInfo(const RuleInfo& ruleInfo, std::set<std::string, std::less<>> tags, std::string title, std::string description, std::size_t line, std::size_t column);
        ScenarioInfo(const FeatureInfo& featureInfo, std::set<std::string, std::less<>> tags, std::string title, std::string description, std::size_t line, std::size_t column);

        [[nodiscard]] const struct FeatureInfo& FeatureInfo() const;
        [[nodiscard]] std::optional<std::reference_wrapper<const struct RuleInfo>> RuleInfo() const;

        [[nodiscard]] const std::set<std::string, std::less<>>& Tags() const;
        [[nodiscard]] const std::string& Title() const;
        [[nodiscard]] const std::string& Description() const;

        [[nodiscard]] const std::filesystem::path& Path() const;

        [[nodiscard]] std::size_t Line() const;
        [[nodiscard]] std::size_t Column() const;

        [[nodiscard]] std::vector<std::unique_ptr<StepInfo>>& Children();
        [[nodiscard]] const std::vector<std::unique_ptr<StepInfo>>& Children() const;

    private:
        std::variant<const struct FeatureInfo*, const struct RuleInfo*> parentInfo;

        std::set<std::string, std::less<>> tags;
        std::string title;
        std::string description;

        std::size_t line;
        std::size_t column;

        std::vector<std::unique_ptr<StepInfo>> children;
    };
}

#endif
