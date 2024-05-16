#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/RuleInfo.hpp"
#include "cucumber-cpp/engine/StepInfo.hpp"
#include <cstddef>
#include <ext/string_conversions.h>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cucumber_cpp::engine
{
    ScenarioInfo::ScenarioInfo(const struct RuleInfo& ruleInfo, std::set<std::string, std::less<>> tags, std::string title, std::string description, std::size_t line, std::size_t column)
        : parentInfo{ &ruleInfo }
        , tags{ std::move(tags) }
        , title{ std::move(title) }
        , description{ std::move(description) }
        , line{ line }
        , column{ column }
    {}

    ScenarioInfo::ScenarioInfo(const struct FeatureInfo& featureInfo, std::set<std::string, std::less<>> tags, std::string title, std::string description, std::size_t line, std::size_t column)
        : parentInfo{ &featureInfo }
        , tags{ std::move(tags) }
        , title{ std::move(title) }
        , description{ std::move(description) }
        , line{ line }
        , column{ column }
    {
    }

    const FeatureInfo& ScenarioInfo::FeatureInfo() const
    {
        if (std::holds_alternative<const struct FeatureInfo*>(parentInfo))
            return *std::get<const struct FeatureInfo*>(parentInfo);

        if (std::holds_alternative<const struct RuleInfo*>(parentInfo))
            return std::get<const struct RuleInfo*>(parentInfo)->FeatureInfo();

        std::abort();
    }

    std::optional<std::reference_wrapper<const struct RuleInfo>> ScenarioInfo::RuleInfo() const
    {
        if (std::holds_alternative<const struct RuleInfo*>(parentInfo))
            return *std::get<const struct RuleInfo*>(parentInfo);

        return std::nullopt;
    }

    // std::variant<std::reference_wrapper<const struct FeatureInfo>, std::reference_wrapper<const struct RuleInfo>> ScenarioInfo::ParentInfo() const
    // {
    //     return parentInfo;
    // }

    const std::set<std::string, std::less<>>& ScenarioInfo::Tags() const
    {
        return tags;
    }

    const std::string& ScenarioInfo::Title() const
    {
        return title;
    }

    const std::string& ScenarioInfo::Description() const
    {
        return description;
    }

    std::size_t ScenarioInfo::Line() const
    {
        return line;
    }

    std::size_t ScenarioInfo::Column() const
    {
        return column;
    }

    std::vector<std::unique_ptr<StepInfo>>& ScenarioInfo::Children()
    {
        return children;
    }

    const std::vector<std::unique_ptr<StepInfo>>& ScenarioInfo::Children() const
    {
        return children;
    }
}
