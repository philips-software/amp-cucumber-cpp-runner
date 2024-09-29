#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/StepInfo.hpp"
#include <cstddef>
#include <cstdlib>
#include <filesystem>
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

    const std::filesystem::path& ScenarioInfo::Path() const
    {
        return FeatureInfo().Path();
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
