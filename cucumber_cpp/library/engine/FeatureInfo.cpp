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
    FeatureInfo::FeatureInfo(std::set<std::string, std::less<>> tags, std::string title, std::string description, std::unique_ptr<struct SourceInfo> sourceInfo, std::size_t line, std::size_t column)
        : tags{ std::move(tags) }
        , title{ std::move(title) }
        , description{ std::move(description) }
        , sourceInfo{ std::move(sourceInfo) }
        , line{ line }
        , column{ column }
    {
    }

    SourceInfo* FeatureInfo::SourceInfo() const
    {
        return sourceInfo.get();
    }


    const std::set<std::string, std::less<>>& FeatureInfo::Tags() const
    {
        return tags;
    }

    const std::string& FeatureInfo::Title() const
    {
        return title;
    }

    const std::string& FeatureInfo::Description() const
    {
        return description;
    }

    const std::filesystem::path& FeatureInfo::Path() const
    {
        return sourceInfo->Path();
    }

    std::size_t FeatureInfo::Line() const
    {
        return line;
    }

    std::size_t FeatureInfo::Column() const
    {
        return column;
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

}
