#include "cucumber-cpp/engine/ScenarioInfo.hpp"
#include "cucumber-cpp/engine/FeatureInfo.hpp"

namespace cucumber_cpp::engine
{
    ScenarioInfo::ScenarioInfo(const struct FeatureInfo& featureInfo, std::vector<std::string> tags, std::string title, std::string description, std::size_t line, std::size_t column)
        : featureInfo{ featureInfo }
        , tags{ std::move(tags) }
        , title{ std::move(title) }
        , description{ std::move(description) }
        , line{ line }
        , column{ column }
    {
    }

    const FeatureInfo& ScenarioInfo::FeatureInfo() const
    {
        return featureInfo;
    }

    const std::vector<std::string>& ScenarioInfo::Tags() const
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

    std::vector<StepInfo>& ScenarioInfo::Children()
    {
        return children;
    }

    const std::vector<StepInfo>& ScenarioInfo::Children() const
    {
        return children;
    }

    std::vector<MissingStepInfo>& ScenarioInfo::MissingChildren()
    {
        return missingChildren;
    }

    const std::vector<MissingStepInfo>& ScenarioInfo::MissingChildren() const
    {
        return missingChildren;
    }

    std::vector<AmbiguousStepInfo>& ScenarioInfo::AmbiguousChildren()
    {
        return ambiguousChildren;
    }

    const std::vector<AmbiguousStepInfo>& ScenarioInfo::AmbiguousChildren() const
    {
        return ambiguousChildren;
    }
}
