#include "cucumber_cpp/library/engine/RuleInfo.hpp"
#include "cucumber_cpp/library/engine/FeatureInfo.hpp"
#include "cucumber_cpp/library/engine/ScenarioInfo.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cucumber_cpp::library::engine
{
    RuleInfo::RuleInfo(const struct FeatureInfo& featureInfo, std::string title, std::string description, std::size_t line, std::size_t column)
        : featureInfo{ featureInfo }
        , title{ std::move(title) }
        , description{ std::move(description) }
        , line{ line }
        , column{ column }
    {}

    const FeatureInfo& RuleInfo::FeatureInfo() const
    {
        return featureInfo;
    }

    const std::string& RuleInfo::Title() const
    {
        return title;
    }

    const std::string& RuleInfo::Description() const
    {
        return description;
    }

    std::size_t RuleInfo::Line() const
    {
        return line;
    }

    std::size_t RuleInfo::Column() const
    {
        return column;
    }

    std::vector<std::unique_ptr<ScenarioInfo>>& RuleInfo::Scenarios()
    {
        return scenarios;
    }

    const std::vector<std::unique_ptr<ScenarioInfo>>& RuleInfo::Scenarios() const
    {
        return scenarios;
    }
}
