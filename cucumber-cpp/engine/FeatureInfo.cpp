#include "cucumber-cpp/engine/FeatureInfo.hpp"
#include "cucumber-cpp/engine/RuleInfo.hpp"
#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::engine
{
    FeatureInfo::FeatureInfo(std::set<std::string, std::less<>> tags, std::string title, std::string description, std::filesystem::path path, std::size_t line, std::size_t column)
        : tags{ std::move(tags) }
        , title{ std::move(title) }
        , description{ std::move(description) }
        , path{ std::move(path) }
        , line{ line }
        , column{ column }
    {
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
        return path;
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
