#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "SourceInfo.hpp"
#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::engine
{
    FeatureInfo::FeatureInfo(cucumber::messages::feature feature, std::unique_ptr<struct SourceInfo> sourceInfo)
        : feature{ std::move(feature) }
        , sourceInfo{ std::move(sourceInfo) }
    {
    }

    SourceInfo* FeatureInfo::SourceInfo() const
    {
        return sourceInfo.get();
    }

    std::set<std::string, std::less<>> FeatureInfo::Tags() const
    {
        const auto range = feature.tags | std::views::transform(&cucumber::messages::tag::name);
        return { range.begin(), range.end() };
    }

    const std::string& FeatureInfo::Title() const
    {
        return feature.name;
    }

    const std::string& FeatureInfo::Description() const
    {
        return feature.description;
    }

    const std::filesystem::path& FeatureInfo::Path() const
    {
        return sourceInfo->Path();
    }

    std::size_t FeatureInfo::Line() const
    {
        return feature.location.line;
    }

    std::size_t FeatureInfo::Column() const
    {
        return feature.location.column.value_or(0);
    }

    std::vector<std::unique_ptr<RuleInfo>>& FeatureInfo::Rules()
    {
        return rules;
    }

    const std::vector<std::unique_ptr<RuleInfo>>& FeatureInfo::Rules() const
    {
        return rules;
    }

    std::vector<std::unique_ptr<ScenarioInfo>>& FeatureInfo::Scenarios()
    {
        return scenarios;
    }

    const std::vector<std::unique_ptr<ScenarioInfo>>& FeatureInfo::Scenarios() const
    {
        return scenarios;
    }

    std::string FeatureInfo::ToJson() const
    {
        return feature.to_json();
    }


}
